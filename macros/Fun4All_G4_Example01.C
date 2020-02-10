#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4detectors/PHG4CylinderSubsystem.h>
#include <g4detectors/PHG4DetectorSubsystem.h>
#include <g4histos/G4HitNtuple.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4ParticleGenerator.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <g4main/PHG4ParticleGun.h>
#include <g4main/HepMCNodeReader.h>
#include <g4main/PHG4Reco.h>
#include <g4main/PHG4TruthSubsystem.h>
#include <g4trackfastsim/PHG4TrackFastSim.h>
#include <g4trackfastsim/PHG4TrackFastSimEval.h>
#include <g4exampledetector/G4Example01Subsystem.h>
#include <phool/recoConsts.h>
#include <phpythia6/PHPythia6.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)
R__LOAD_LIBRARY(libg4detectors.so)
R__LOAD_LIBRARY(libg4example01detector.so)
R__LOAD_LIBRARY(libg4histos.so)
R__LOAD_LIBRARY(libPHPythia6.so)
R__LOAD_LIBRARY(libg4trackfastsim.so)

#endif

void Fun4All_G4_Example01(int nEvents = 1)
{

  gSystem->Load("libfun4all");
  gSystem->Load("libg4detectors");
  gSystem->Load("libg4testbench");
  gSystem->Load("libg4histos");
  gSystem->Load("libg4example01detector.so");
  gSystem->Load("libg4trackfastsim.so");

  ///////////////////////////////////////////
  // Make the Server
  //////////////////////////////////////////
  Fun4AllServer *se = Fun4AllServer::instance();
  recoConsts *rc = recoConsts::instance();
  // if you want to fix the random seed to reproduce results
  // set this flag
  // nail this down so I know what the first event looks like...
  rc->set_IntFlag("RANDOMSEED",12345); 

  //
  // Particle Generator
  //
  bool particle = true;
  bool setgun = false;
  bool pythia6 = false;

  // the PHG4ParticleGenerator makes cones using phi and eta
  if (particle) {
    PHG4ParticleGenerator *gen = new PHG4ParticleGenerator("PGENERATOR");
    gen->set_name("e-");
    gen->set_vtx(0, 0, 0);
    gen->set_eta_range(-0.9, 0.9);
    gen->set_mom_range(1.0, 10.0);
    gen->set_z_range(0.,0.);
    gen->set_phi_range(-5/180.*TMath::Pi(), 5/180.*TMath::Pi());
    se->registerSubsystem(gen);
  }

  else if (setgun) {
    PHG4ParticleGun *gun = new PHG4ParticleGun();
    gun->set_name("pi-");
    //gun->set_name("geantino");
    //gun->set_name("proton");
    gun->set_vtx(0, -5, -20); // shoots right into the original Examle01 volume
    gun->set_mom(0, 0, 1);
    se->registerSubsystem(gun);
  }
  else if (pythia6) {
    gSystem->Load("libPHPythia6.so");

    PHPythia6 *pythia6 = new PHPythia6();
    // see coresoftware/generators/PHPythia6 for example config
    pythia6->set_config_file("phpythia6_ep.cfg");
    se->registerSubsystem(pythia6);
  }

  //
  // Geant4 setup
  //
  PHG4Reco* g4Reco = new PHG4Reco();
  g4Reco->set_field(4); // no field
  // try non default physics lists
  //g4Reco->SetPhysicsList("FTFP_BERT_HP");

  G4Example01Subsystem *example01 = new G4Example01Subsystem("HoleInBox");
  g4Reco->registerSubsystem(example01);

  //g4Reco->set_rapidity_coverage(4); // according to drawings

  double si_thickness[6] = {0.02, 0.02, 0.0625, 0.032, 0.032, 0.032};
  double svxrad[6] = {2.9238, 5.38772, 7.82395, 10.8585, 13.2827, 15.7054};
  double length[6] = {40., 40., 40., 40., 40., 40.};  // -1 use eta coverage to determine length
  PHG4CylinderSubsystem *cyl;
  // here is our silicon:
  for (int ilayer = 0; ilayer < 5; ilayer++)
  {
    cyl = new PHG4CylinderSubsystem("SVTX", ilayer);
    cyl->set_double_param("radius", svxrad[ilayer]);
    cyl->set_string_param("material", "G4_Si");
    cyl->set_double_param("thickness", si_thickness[ilayer]);
    cyl->SetActive();
    cyl->SuperDetector("SVTX");
    if (length[ilayer] > 0)
    {
      cyl->set_int_param("lengthviarapidity",0);
      cyl->set_double_param("length", length[ilayer]);
    }
    g4Reco->registerSubsystem(cyl);
  }
  // Black hole swallows everything - prevent loopers from returning
  // to inner detectors
  cyl = new PHG4CylinderSubsystem("BlackHole", 0);
  cyl->set_double_param("radius", 150);        // 80 cm
  cyl->set_double_param("thickness", 0.1); // does not matter (but > 0)
  cyl->SetActive();
  cyl->BlackHole(); // eats everything
  g4Reco->registerSubsystem(cyl);

  // read-in HepMC events to Geant4 if there is any
  HepMCNodeReader *hr = new HepMCNodeReader();
  se->registerSubsystem(hr);
  //g4Reco->save_DST_geometry(false); // saving the geometry crashes here
  
  PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);

  se->registerSubsystem( g4Reco );

  ///////////////////////////////////////////
  // Fun4All modules
  ///////////////////////////////////////////

  G4HitNtuple *hits = new G4HitNtuple("Hits");
  hits->AddNode("HoleInBox",0);
  se->registerSubsystem(hits);
  
  //---------------------------
  // fast pattern recognition and full Kalman filter
  // output evaluation file for truth track and reco tracks are PHG4TruthInfoContainer
  //---------------------------
  PHG4TrackFastSim *kalman = new PHG4TrackFastSim("PHG4TrackFastSim");
  kalman->set_use_vertex_in_fitting(false);
  //kalman->set_sub_top_node_name("SVTX");
  //kalman->set_trackmap_out_name("SvtxTrackMap");
  kalman->set_sub_top_node_name("HoleInBox");
  kalman->set_trackmap_out_name("HoleInBoxTrackMap");

  //  add Si Trtacker
  kalman->add_phg4hits(
      "G4HIT_HoleInBox",                //      const std::string& phg4hitsNames,
      PHG4TrackFastSim::Cylinder,  //      const DETECTOR_TYPE phg4dettype,
      300e-4,                      //       radial-resolution [cm]
      300e-4,                       //        azimuthal-resolution [cm]
      1,                           //      z-resolution [cm]
      1,                           //      efficiency,
      0                            //      noise hits
  );
  se->registerSubsystem(kalman);

  PHG4TrackFastSimEval *fast_sim_eval = new PHG4TrackFastSimEval("FastTrackingEval",
      "FastTrackingEval_HoleInBox.root",
      "HoleInBoxTrackMap"
      );
  //fast_sim_eval->set_filename("FastTrackingEval.root");
  se->registerSubsystem(fast_sim_eval);
  //---------------------------

  ///////////////////////////////////////////
  // IOManagers...
  ///////////////////////////////////////////

  // Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT","G4Example01.root");
  // out->Verbosity(10);
  // se->registerOutputManager(out);

  // this (dummy) input manager just drives the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager( "Dummy");
  se->registerInputManager( in );
  // events = 0 => run forever
  if (nEvents <= 0)
  {
    return 0;
  }
  se->run(nEvents);
  example01->Print();
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
}
/* vim: set expandtab shiftwidth=2 tabstop=2: */
