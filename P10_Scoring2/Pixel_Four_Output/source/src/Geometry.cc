//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Geometry.cc
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "Geometry.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4NistManager.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

#include "SensitiveVolume.hh"
#include "G4SDManager.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"

//-- Global objects ------------------------------------------------------------
G4ThreadLocal G4GlobalMagFieldMessenger* Geometry::fMagFieldMessenger = nullptr;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
  Geometry::Geometry() {}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
  Geometry::~Geometry() {}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
  G4VPhysicalVolume* Geometry::Construct()
//------------------------------------------------------------------------------
{
// Get the pointer to 'Material Manager'
   G4NistManager* materi_Man = G4NistManager::Instance();

// Define 'World Volume'
   // Define the shape of solid
   G4double leng_X_World = 2.0*m;         // X-full-length of world
   G4double leng_Y_World = 2.0*m;         // Y-full-length of world
   G4double leng_Z_World = 2.0*m;         // Z-full-length of world
   auto solid_World = new G4Box{ "Solid_World",
                         leng_X_World/2.0, leng_Y_World/2.0, leng_Z_World/2.0 };

   // Define the logical volume
   G4Material* materi_World = materi_Man->FindOrBuildMaterial( "G4_AIR" );
   auto logVol_World = new G4LogicalVolume{ solid_World, materi_World,
                                            "LogVol_World" };
   // logVol_World->SetVisAttributes ( G4VisAttributes::GetInvisible() );

   // Placement of the logical volume
   G4int copyNum_World = 0;               // Set ID number of world
   auto physVol_World  = new G4PVPlacement{ G4Transform3D(), "PhysVol_World",
                                        logVol_World, 0, false, copyNum_World };
   G4double absorber_Z = 20*mm;
   G4double absorber_X = 20*mm;
   G4double absorber_Y = 15*mm;
   G4double absorber_pos_X = 0.0*cm;
   G4double absorber_ang = 0.0*deg;
   G4double absorber_pos_Y = 0.0*cm;
   G4double absorber_pos_Z = 0.0*cm;
   auto solid_absorber = new G4Box{ "Solid_absorber",
                         absorber_X/2.0, absorber_Y/2.0, absorber_Z/2.0 };
   G4Material* materi_absorber = materi_Man->FindOrBuildMaterial( "G4_W" );
   auto logVol_absorber = new G4LogicalVolume{ solid_absorber, materi_absorber,
                                            "LogVol_absorber" };

// Define 'Pixel Detector' - Global Envelop
   // Define the shape of the global envelop
   G4double leng_X_PixEnvG = 15.0*mm;     // X-full-length of pixel: global envelop
   G4double leng_Y_PixEnvG = 20.0*mm;     // Y-full-length of pixel: global envelop
   G4double leng_Z_PixEnvG =  0.2*mm;     // Z-full-length of pixel: global envelop
   auto solid_PixEnvG = new G4Box{ "Solid_PixEnvG", leng_X_PixEnvG/2.0,
                                   leng_Y_PixEnvG/2.0, leng_Z_PixEnvG/2.0 };

   // Define the logical volume of the global envelop
   G4Material* materi_PixEnvG = materi_Man->FindOrBuildMaterial( "G4_AIR" );
   auto logVol_PixEnvG = new G4LogicalVolume{ solid_PixEnvG, materi_PixEnvG,
                                              "LogVol_PixEnvG" };
   // logVol_PixEnvG->SetVisAttributes ( G4VisAttributes::GetInvisible() );

// Define 'Pixel Detector' - Local Envelop (divided the global envelop in Y-direction)
   // Define the shape of the local envelop
   G4int nDiv_Y = 10;                                 // Number of divisions in Y-direction
   G4double leng_X_PixEnvL = leng_X_PixEnvG;          // X-full-length of pixel: local envelop
   G4double leng_Y_PixEnvL = leng_Y_PixEnvG/nDiv_Y;   // Y-full-length of pixel: local envelop
   G4double leng_Z_PixEnvL = leng_Z_PixEnvG;          // Z-full-length of pixel: local envelop
   auto solid_PixEnvL = new G4Box{ "Solid_PixEnvL", leng_X_PixEnvL/2.0,
                                   leng_Y_PixEnvL/2.0, leng_Z_PixEnvL/2.0 };

   // Define the logical volume of the local envelop
   G4Material* materi_PixEnvL = materi_Man->FindOrBuildMaterial( "G4_AIR" );
   auto logVol_PixEnvL = new G4LogicalVolume{ solid_PixEnvL, materi_PixEnvL,
                                              "LogVol_PixEnvL" };
   //logVol_PixEnvL->SetVisAttributes ( G4VisAttributes::GetInvisible() );

   // Placement of the local envelop to the global envelop using Replica
   new G4PVReplica{ "PhysVol_PixEnvL", logVol_PixEnvL, logVol_PixEnvG, kYAxis,
                    nDiv_Y, leng_Y_PixEnvL };

// Define 'Pixel Detector' - Pixel Element (divided the local envelop in X-direction)
   // Define the shape of the pixel element
   G4int nDiv_X = 5;                                   // Number of divisions in Y-direction
   G4double leng_X_PixElmt = leng_X_PixEnvG/nDiv_X;    // X-full-length of pixel: pixel element
   G4double leng_Y_PixElmt = leng_Y_PixEnvG/nDiv_Y;    // Y-full-length of pixel: pixel element
   G4double leng_Z_PixElmt = leng_Z_PixEnvG;           // Z-full-length of pixel: pixel element
   auto solid_PixElmt = new G4Box{ "Solid_PixElmt", leng_X_PixElmt/2.0,
                                   leng_Y_PixElmt/2.0, leng_Z_PixElmt/2.0 };

   // Define the logical volume of the pixel element
   G4Material* materi_PixElmt = materi_Man->FindOrBuildMaterial( "G4_SILICON_DIOXIDE" );
   auto logVol_PixElmt = new G4LogicalVolume{ solid_PixElmt, materi_PixElmt,
                                              "LogVol_PixElmt" };
   //logVol_PixElmt->SetVisAttributes ( G4VisAttributes::GetInvisible() );

   // Placement of the pixel elements to the local envelop using Replica
   new G4PVReplica{ "PhysVol_PixElmt", logVol_PixElmt, logVol_PixEnvL, kXAxis,
                    nDiv_X, leng_X_PixElmt };

// Placement of the 'Pixel Detectors' to the world: Put the 'global envelop'
   // Z positions and setting angles of four detectors
   G4double z_pos[4] = { -7.5*cm, -2.5*cm, +2.5*cm, +7.5*cm };
   G4double z_ang[4] = { 0.0*deg, 0.0*deg, 0.0*deg, 0.0*deg };

   // Placment of the detectors
   for (G4int id=0; id<4; id++){
     G4double pos_X_LogV_PixEnvG = 0.0*cm;       // X-location LogV_PixEnvG
     G4double pos_Y_LogV_PixEnvG = 0.0*cm;       // Y-location LogV_PixEnvG
     G4double pos_Z_LogV_PixEnvG = z_pos[id];    // Z-location LogV_PixEnvG
     auto threeVect_LogV_PixEnvG = G4ThreeVector{ pos_X_LogV_PixEnvG,
                                       pos_Y_LogV_PixEnvG, pos_Z_LogV_PixEnvG };
     auto rotMtrx_LogV_PixEnvG = G4RotationMatrix{};
     rotMtrx_LogV_PixEnvG.rotateZ( z_ang[id] );
     auto trans3D_LogV_PixEnvG = G4Transform3D{ rotMtrx_LogV_PixEnvG, threeVect_LogV_PixEnvG };

     G4int copyNum_LogV_PixEnvG = 1000 + id;                // Set ID number of LogV_PixEnvG
     new G4PVPlacement{ trans3D_LogV_PixEnvG, "PhysVol_PixEnvG", logVol_PixEnvG, physVol_World,
                        false, copyNum_LogV_PixEnvG, true };
   }
   for (G4int id = 0; id<4; id++){
      G4double pos_X_LogV_absorber = absorber_pos_X;
      G4double pos_Y_LogV_absorber = absorber_pos_Y;
      G4double pos_Z_LogV_absorber = z_pos[id] + leng_Z_PixEnvG/2.0 + absorber_Z/2.0;
      auto threeVect_LogV_absorber = G4ThreeVector{ pos_X_LogV_absorber,
                                       pos_Y_LogV_absorber, pos_Z_LogV_absorber };
      auto rotMtrx_LogV_absorber = G4RotationMatrix{};
      rotMtrx_LogV_absorber.rotateZ( absorber_ang );
      auto trans3D_LogV_absorber = G4Transform3D{ rotMtrx_LogV_absorber, threeVect_LogV_absorber };

      G4int copyNum_LogV_absorber = 2000 +id ;                // Set ID number of LogV_absorber
      new G4PVPlacement{ trans3D_LogV_absorber, "PhysVol_absorber", logVol_absorber, physVol_World,
                        false, copyNum_LogV_absorber, true };
   }

// Return the physical world
   return physVol_World;
}
//------------------------------------------------------------------------------
void Geometry::ConstructSDandField()
//------------------------------------------------------------------------------
{
// Add uniform magnetic field to the world
   // auto fieldValue = G4ThreeVector{ 1.0*tesla, 0.0, 0.0 };
   // fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
   // fMagFieldMessenger->SetVerboseLevel(1);

// Register the field messenger for deleting
   // G4AutoDelete::Register(fMagFieldMessenger);

//Sensitive Volume

SensitiveVolume* aSensiVol = new SensitiveVolume{"Pixel"} ;
aSensiVol->SetSimData(simdata) ;
auto SDMan =  G4SDManager::GetSDMpointer();
SDMan->AddNewDetector(aSensiVol);
SetSensitiveDetector("LogVol_PixElmt", aSensiVol, true );

}

