#include "G4Example01Detector.h"

#include <g4main/PHG4Detector.h>  // for PHG4Detector

#include <Geant4/G4Box.hh>
#include <Geant4/G4Color.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4SubtractionSolid.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4UnionSolid.hh>
#include <Geant4/G4VisAttributes.hh>

#include <cmath>
#include <iostream>  // for operator<<, endl, bas...

class G4VSolid;
class PHCompositeNode;

using namespace std;

G4Example01Detector::G4Example01Detector(PHG4Subsystem *subsys, PHCompositeNode *Node, const std::string &dnam)
  : PHG4Detector(subsys, Node, dnam)
{
}

//_______________________________________________________________
//_______________________________________________________________
int G4Example01Detector::IsInDetector(G4VPhysicalVolume *volume) const
{
  set<G4VPhysicalVolume *>::const_iterator iter = m_PhysicalVolumesSet.find(volume);
  if (iter != m_PhysicalVolumesSet.end())
  {
    return 1;
  }

  return 0;
}

void G4Example01Detector::ConstructMe(G4LogicalVolume *logicWorld)
{
  double prapidity = 1;
  int fNbOfTiles=6;
  
  double bmt_length = (1-exp(-2*prapidity))/exp(-prapidity)*80*cm;
  double bmt_thickness[6] = {5*mm, 5*mm, 5*mm, 5*mm, 5*mm, 5*mm};
  double bmt_rad[6] = {15*cm, 28*cm, 41*cm, 54*cm, 67*cm, 80*cm};
  
  for (G4int copyNo=0; copyNo<fNbOfTiles; copyNo++) {
  
    G4Tubs* BMTTileS
      = new G4Tubs("BMTTile_Solid", bmt_rad[copyNo]-bmt_thickness[copyNo]/2, bmt_rad[copyNo]+bmt_thickness[copyNo]/2, bmt_length/2, 0.*deg, 360.*deg);

    //fLogicBMTTile[copyNo] =
    G4LogicalVolume* fLogicBMTTile = 
            new G4LogicalVolume(BMTTileS, G4Material::GetMaterial("G4_Ar"), "BMTTile_LV",0,0,0);

    G4VisAttributes *vis = new G4VisAttributes(G4Color(G4Colour::Grey())); // grey is good to see the tracks in the display
    vis->SetForceSolid(true);
    fLogicBMTTile->SetVisAttributes(vis);

    G4VPhysicalVolume *phy = new G4PVPlacement(0,                            // no rotation
                                               G4ThreeVector(0,0,0), // at (x,y,z)
                                               fLogicBMTTile,        // its logical volume
                                               "BMTTile_PV",                 // its name
                                               logicWorld,                    // its mother  volume
                                               false,                        // no boolean operations
                                               copyNo,                       // copy number
                                               OverlapCheck());              // checking overlaps 
// add it to the list of placed volumes so the IsInDetector method
// picks them up
    m_PhysicalVolumesSet.insert(phy);
  }
  return;
}

void G4Example01Detector::Print(const std::string &what) const
{
  cout << "Example01 Detector:" << endl;
  if (what == "ALL" || what == "VOLUME")
  {
    cout << "Version 0.1" << endl;
  }
  return;
}
/* vim: set expandtab shiftwidth=2 tabstop=2: */
