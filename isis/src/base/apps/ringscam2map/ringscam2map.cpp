#define GUIHELPERS

#include "Isis.h"
#include "Distance.h"
#include "Camera.h"
#include "ProjectionFactory.h"
#include "ProcessRubberSheet.h"
#include "IException.h"
#include "RingPlaneProjection.h"
#include "ringscam2map.h"
#include "Pvl.h"
#include "IString.h"
#include "PushFrameCameraDetectorMap.h"
#include "Target.h"

using namespace std;
using namespace Isis;

void PrintMap();
void LoadMapRes();
void LoadCameraRes();
void LoadMapRange();
void LoadCameraRange();

map <QString, void *> GuiHelpers() {
  map <QString, void *> helper;
  helper ["PrintMap"] = (void *) PrintMap;
  helper ["LoadMapRes"] = (void *) LoadMapRes;
  helper ["LoadCameraRes"] = (void *) LoadCameraRes;
  helper ["LoadMapRange"] = (void *) LoadMapRange;
  helper ["LoadCameraRange"] = (void *) LoadCameraRange;
  return helper;
}


// Global variables
void BandChange(const int band);
Cube *icube;
Camera *incam;

void IsisMain() {
  // We will be warping a cube
  ProcessRubberSheet p;

  // Get the map projection file provided by the user
  UserInterface &ui = Application::GetUserInterface();
  Pvl userMap;
  userMap.Read(ui.GetFileName("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping", Pvl::Traverse);

  // Open the input cube and get the camera
  icube = p.SetInputCube("FROM");
  incam = icube->camera();

  // Make sure it is not the sky
  if(incam->target()->isSky()) {
    QString msg = "The image [" + ui.GetFileName("FROM") +
                  "] is targeting the sky, use skymap instead.";
    throw IException(IException::User, msg, _FILEINFO_);
  }

  // Get the mapping group from the camera
  Pvl camMap;
  incam->basicRingMapping(camMap);
  PvlGroup &camGrp = camMap.FindGroup("Mapping");

  // Make the target info match the user mapfile
  double minrad, maxrad, minaz, maxaz;
  incam->ringRange(minrad, maxrad, minaz, maxaz, userMap);
  camGrp.AddKeyword(PvlKeyword("MinimumRadius", toString(minrad)), Pvl::Replace);
  camGrp.AddKeyword(PvlKeyword("MaximumRadius", toString(maxrad)), Pvl::Replace);
  camGrp.AddKeyword(PvlKeyword("MinimumAzimuth", toString(minaz)), Pvl::Replace);
  camGrp.AddKeyword(PvlKeyword("MaximumAzimuth", toString(maxaz)), Pvl::Replace);

  // We want to delete the keywords we just added if the user wants the range
  // out of the mapfile, otherwise they will replace any keywords not in the
  // mapfile
  if (ui.GetString("DEFAULTRANGE") == "MAP" || ui.GetBoolean("MATCHMAP")) {
    camGrp.DeleteKeyword("MinimumRadius");
    camGrp.DeleteKeyword("MaximumRadius");
    camGrp.DeleteKeyword("MinimumAzimuth");
    camGrp.DeleteKeyword("MaximumAzimuth");
  }
  // Otherwise, remove the keywords from the map file so the camera keywords
  // will be propogated correctly
  else {
    while(userGrp.HasKeyword("MinimumRadius")) {
      userGrp.DeleteKeyword("MinimumRadius");
    }
    while(userGrp.HasKeyword("MinimumAzimuth")) {
      userGrp.DeleteKeyword("MinimumAzimuth");
    }
    while(userGrp.HasKeyword("MaximumRadius")) {
      userGrp.DeleteKeyword("MaximumRadius");
    }
    while(userGrp.HasKeyword("MaximumAzimuth")) {
      userGrp.DeleteKeyword("MaximumAzimuth");
    }
  }

  // If the user decided to enter a ground range then override
  if(ui.WasEntered("MINAZ") && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("MinimumAzimuth",
                                  toString(ui.GetDouble("MINAZ"))), Pvl::Replace);
  }

  if(ui.WasEntered("MAXAZ") && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("MaximumAzimuth",
                                  toString(ui.GetDouble("MAXAZ"))), Pvl::Replace);
  }

  if(ui.WasEntered("MINRAD") && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("MinimumRadius",
                                  toString(ui.GetDouble("MINRAD"))), Pvl::Replace);
  }

  if(ui.WasEntered("MAXRAD") && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("MaximumRadius",
                                  toString(ui.GetDouble("MAXRAD"))), Pvl::Replace);
  }

  // If they want the res. from the mapfile, delete it from the camera so
  // nothing gets overridden
  if(ui.GetString("PIXRES") == "MAP" || ui.GetBoolean("MATCHMAP")) {
    camGrp.DeleteKeyword("PixelResolution");
  }
  // Otherwise, delete any resolution keywords from the mapfile so the camera
  // info is propagated over
  else if(ui.GetString("PIXRES") == "CAMERA") {
    if(userGrp.HasKeyword("Scale")) {
      userGrp.DeleteKeyword("Scale");
    }
    if(userGrp.HasKeyword("PixelResolution")) {
      userGrp.DeleteKeyword("PixelResolution");
    }
  }

  // Copy any defaults that are not in the user map from the camera map file
  for(int k = 0; k < camGrp.Keywords(); k++) {
    if(!userGrp.HasKeyword(camGrp[k].Name())) {
      userGrp += camGrp[k];
    }
  }

  // If the user decided to enter a resolution then override
  if(ui.GetString("PIXRES") == "MPP" && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("PixelResolution",
                                  toString(ui.GetDouble("RESOLUTION"))),
                       Pvl::Replace);
    if(userGrp.HasKeyword("Scale")) {
      userGrp.DeleteKeyword("Scale");
    }
  }
  else if(ui.GetString("PIXRES") == "PPD" && !ui.GetBoolean("MATCHMAP")) {
    userGrp.AddKeyword(PvlKeyword("Scale",
                                  toString(ui.GetDouble("RESOLUTION"))),
                       Pvl::Replace);
    if(userGrp.HasKeyword("PixelResolution")) {
      userGrp.DeleteKeyword("PixelResolution");
    }
  }

  // See if the user want us to handle the azimuth seam
  if((ui.GetString("DEFAULTRANGE") == "CAMERA" || ui.GetString("DEFAULTRANGE") == "MINIMIZE") &&
      !ui.GetBoolean("MATCHMAP")) {
    //TODO This camera method will need attention for rings***** Solution:  just call ringRange directly
    // if(incam->IntersectsLongitudeDomain(userMap)) {
    if (incam->ringRange(minrad, maxrad, minaz, maxaz, userMap)) {
      if(ui.GetString("AZSEAM") == "AUTO") {
        if((int) userGrp["AzimuthDomain"] == 360) {
          userGrp.AddKeyword(PvlKeyword("AzimuthDomain", "180"),
                             Pvl::Replace);
          if(incam->ringRange(minrad, maxrad, minaz, maxaz, userMap)) {
            // Its looks like a global image so switch back to the
            // users preference
            userGrp.AddKeyword(PvlKeyword("AzimuthDomain", "360"),
                               Pvl::Replace);
          }
        }
        else {
          userGrp.AddKeyword(PvlKeyword("AzimuthDomain", "360"),
                             Pvl::Replace);
          if(incam->ringRange(minrad, maxrad, minaz, maxaz, userMap)) {
            // Its looks like a global image so switch back to the
            // users preference
            userGrp.AddKeyword(PvlKeyword("AzimuthDomain", "180"),
                               Pvl::Replace);
          }
        }
        // Make the target info match the new azimuth domain
        // Use radius for where camera expects latitude & azimuth where the camera expects longitude
        double minrad, maxrad, minaz, maxaz;
        incam->ringRange(minrad, maxrad, minaz, maxaz, userMap);
        if(!ui.WasEntered("MINRAD")) {
          userGrp.AddKeyword(PvlKeyword("MinimumRadius", toString(minrad)), Pvl::Replace);
        }
        if(!ui.WasEntered("MAXRAD")) {
          userGrp.AddKeyword(PvlKeyword("MaximumRadius", toString(maxrad)), Pvl::Replace);
        }
        if(!ui.WasEntered("MINAZ")) {
          userGrp.AddKeyword(PvlKeyword("MinimumAzimuth", toString(minaz)), Pvl::Replace);
        }
        if(!ui.WasEntered("MAXAZ")) {
          userGrp.AddKeyword(PvlKeyword("MaximumAzimuth", toString(maxaz)), Pvl::Replace);
        }
      }

      else if(ui.GetString("AZSEAM") == "ERROR") {
        QString msg = "The image [" + ui.GetFileName("FROM") + "] crosses the " +
                     "azimuth seam";
        throw IException(IException::User, msg, _FILEINFO_);
      }
    }
  }

  // Use the updated label to create the output projection
  int samples, lines;
  RingPlaneProjection *outmap;
  bool trim;

  // Make sure shape model is  ring plane
  if (incam->target()->shape()->name() == "Plane") {
    // Determine the image size
    if(ui.GetString("DEFAULTRANGE") == "MINIMIZE" && !ui.GetBoolean("MATCHMAP")) {
      outmap = (RingPlaneProjection *) ProjectionFactory::RingsCreateForCube(userMap, samples, lines, *incam);
      trim = false;
    }
    else if(ui.GetString("DEFAULTRANGE") == "CAMERA" && !ui.GetBoolean("MATCHMAP")) {
      outmap =  (RingPlaneProjection *) ProjectionFactory::RingsCreateForCube(userMap, samples, lines, false);
      trim = ui.GetBoolean("TRIM");
    }
    else { // DEFAULTRANGE = MAP
      outmap =  (RingPlaneProjection *) ProjectionFactory::RingsCreateForCube(userMap, samples, lines,
               ui.GetBoolean("MATCHMAP"));
      trim = ui.GetBoolean("TRIM");
    }
  }
  else {
    QString msg = "The image [" + ui.GetFileName("FROM") + " does not have a plane shape. " +
                     " Run spiceinit with shape=ringplane";
        throw IException(IException::User, msg, _FILEINFO_);
  }

  // Output the mapping group used to the Gui session log
  PvlGroup cleanMapping = outmap->Mapping();
  Application::GuiLog(cleanMapping);

  // Allocate the output cube and add the mapping labels
  Cube *ocube = p.SetOutputCube("TO", samples, lines, icube->bandCount());

  ocube->putGroup(cleanMapping);

  // Set up the interpolator
  Interpolator *interp = NULL;
  if(ui.GetString("INTERP") == "NEARESTNEIGHBOR") {
    interp = new Interpolator(Interpolator::NearestNeighborType);
  }
  else if(ui.GetString("INTERP") == "BILINEAR") {
    interp = new Interpolator(Interpolator::BiLinearType);
  }
  else if(ui.GetString("INTERP") == "CUBICCONVOLUTION") {
    interp = new Interpolator(Interpolator::CubicConvolutionType);
  }

  // See if we need to deal with band dependent camera models
  if(!incam->IsBandIndependent()) {
    p.BandChange(BandChange);
  }

  //  See if center of input image projects.  If it does, force tile
  //  containing this center to be processed in ProcessRubberSheet.
  //  TODO:  WEIRD ... why is this needed ... Talk to Tracie ... JAA??
  double centerSamp = icube->sampleCount() / 2.;
  double centerLine = icube->lineCount() / 2.;
  if(incam->SetImage(centerSamp, centerLine)) {
    // Force rings data into Isis by returning radius for latitude and azimuth for longitude
    // if(outmap->SetUniversalGround(incam->UniversalLatitude(),
    //                               incam->UniversalLongitude())) {
    if(outmap->SetUniversalGround(incam->LocalRadius().meters(),
                                  incam->UniversalLongitude())) {
      p.ForceTile(outmap->WorldX(), outmap->WorldY());
    }
  }
  // Create an alpha cube group for the output cube
  if(!ocube->hasGroup("AlphaCube")) {
    PvlGroup alpha("AlphaCube");
    alpha += PvlKeyword("AlphaSamples", toString(icube->sampleCount()));
    alpha += PvlKeyword("AlphaLines", toString(icube->lineCount()));
    alpha += PvlKeyword("AlphaStartingSample", toString(0.5));
    alpha += PvlKeyword("AlphaStartingLine", toString(0.5));
    alpha += PvlKeyword("AlphaEndingSample", toString(icube->sampleCount() + 0.5));
    alpha += PvlKeyword("AlphaEndingLine", toString(icube->lineCount() + 0.5));
    alpha += PvlKeyword("BetaSamples", toString(icube->sampleCount()));
    alpha += PvlKeyword("BetaLines", toString(icube->lineCount()));
    ocube->putGroup(alpha);
  }

  // We will need a transform class
  Transform *transform = 0;
  
  // Okay we need to decide how to apply the rubbersheeting for the transform
  // Does the user want to define how it is done?
  if (ui.GetString("WARPALGORITHM") == "FORWARDPATCH") {
    transform = new ringscam2mapForward(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);

    if (ui.WasEntered("PATCHSIZE")) {
      int patchSize = ui.GetInteger("PATCHSIZE");
      if (patchSize <= 1) patchSize = 3; // Make the patchsize reasonable
      p.setPatchParameters(1, 1, patchSize, patchSize, 
                           patchSize-1, patchSize-1);
    }

    p.processPatchTransform(*transform, *interp);
  }

  else if (ui.GetString("WARPALGORITHM") == "REVERSEPATCH") {
    transform = new ringscam2mapReverse(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);

    if (ui.WasEntered("PATCHSIZE")) {
      int patchSize = ui.GetInteger("PATCHSIZE");
      int minPatchSize = 4;
      if (patchSize < minPatchSize) minPatchSize = patchSize;
      p.SetTiling(patchSize, minPatchSize);
    }

    p.StartProcess(*transform, *interp);
  }

  // The user didn't want to override the program smarts.
  // Handle framing cameras.  Always process using the backward
  // driven system (tfile).
  else if (incam->GetCameraType() == Camera::Framing) {
    transform = new ringscam2mapReverse(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);
    p.SetTiling(4, 4);
    p.StartProcess(*transform, *interp);
  }

  // The user didn't want to override the program smarts.
  // Handle linescan cameras.  Always process using the forward
  // driven patch option. Faster and we get better orthorectification
  // 
  // TODO:  For now use the default patch size.  Need to modify
  // to determine patch size based on 1) if the limb is in the file
  // or 2) if the DTM is much coarser than the image
  else if (incam->GetCameraType() == Camera::LineScan) {
    transform = new ringscam2mapForward(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);

    p.processPatchTransform(*transform, *interp);
  }

  // The user didn't want to override the program smarts.
  // Handle pushframe cameras.  Always process using the forward driven patch 
  // option.  It is much faster than the tfile method.  We will need to 
  // determine patch sizes based on the size of the push frame.
  // 
  // TODO: What if the user has run crop, enlarge, or shrink on the push
  // frame cube.  Things probably won't work unless they do it just right
  // TODO: What about the THEMIS VIS Camera.  Will tall narrow (128x4) patches
  // work okay?
  else if (incam->GetCameraType() == Camera::PushFrame) {
    transform = new ringscam2mapForward(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);

    // Get the frame height
    PushFrameCameraDetectorMap *dmap = (PushFrameCameraDetectorMap *) incam->DetectorMap();
    int frameSize = dmap->frameletHeight() / dmap->LineScaleFactor();

    // Check for even/odd cube to determine starting line
    PvlGroup &instGrp = icube->label()->FindGroup("Instrument", Pvl::Traverse);
    int startLine = 1;

    // Get the alpha cube group in case they cropped the image
    AlphaCube acube(*icube->label());
    double betaLine = acube.AlphaLine(1.0);
    if (fabs(betaLine - 1.0) > 0.0000000001) {
      if (fabs(betaLine - (int) betaLine) > 0.00001) {
        string msg = "Input file is a pushframe camera cropped at a ";
        msg += "fractional pixel.  Can not project"; 
        throw IException(IException::User, msg, _FILEINFO_);
      }
      int offset = (((int) (betaLine + 0.5)) - 1) % frameSize;
      startLine -= offset;
    }

    if (((QString)instGrp["Framelets"]).toUpper() == "EVEN") {
      startLine += frameSize;
    }

    p.setPatchParameters(1, startLine, 5, frameSize,
                         4, frameSize * 2);

    p.processPatchTransform(*transform, *interp);
  }

  // The user didn't want to override the program smarts.  The other camera 
  // types have not be analyized.  This includes Radar and Point.  Continue to
  // use the reverse geom option with the default tiling hints
  else {
    transform = new ringscam2mapReverse(icube->sampleCount(),
                                   icube->lineCount(), incam, samples,lines,
                                   outmap, trim);
                                   // (Planar*)outmap, trim);

    int tileStart, tileEnd;
    incam->GetGeometricTilingHint(tileStart, tileEnd);
    p.SetTiling(tileStart, tileEnd);

    p.StartProcess(*transform, *interp);
  }

  // Wrap up the warping process 
  p.EndProcess();

  // add mapping to print.prt
  Application::Log(cleanMapping);

  // Cleanup
  delete outmap;
  delete transform;
  delete interp;
}

// Transform object constructor
ringscam2mapForward::ringscam2mapForward(const int inputSamples,
                                         const int inputLines,
                                         Camera *incam, const int outputSamples,
                                         const int outputLines,
                                         RingPlaneProjection *outmap, bool trim) {
                                         // Planar *outmap, bool trim) {
  p_inputSamples = inputSamples;
  p_inputLines = inputLines;
  p_incam = incam;

  p_outputSamples = outputSamples;
  p_outputLines = outputLines;
  p_outmap = outmap;

  p_trim = trim;
}

// Transform method mapping input line/samps to radii/lons to output line/samps
bool ringscam2mapForward::Xform(double &outSample, double &outLine,
                    const double inSample, const double inLine) {

  // See if the input image coordinate converts to a radius/lon
  if (!p_incam->SetImage(inSample,inLine)) return false;

  // Does that ground coordinate work in the map projection
  // We will be forcing ring data to work by substituting radius for latitude & azimuth for longitude
  double rad = p_incam->UniversalLatitude();
  double az = p_incam->UniversalLongitude();
  if(!p_outmap->SetUniversalGround(rad,az)) return false;

  // See if we should trim
  if((p_trim) && (p_outmap->HasGroundRange())) {
    if(p_outmap->Radius() < p_outmap->MinimumRadius()) return false;
    if(p_outmap->Radius() > p_outmap->MaximumRadius()) return false;
    if(p_outmap->Azimuth() < p_outmap->MinimumAzimuth()) return false;
    if(p_outmap->Azimuth() > p_outmap->MaximumAzimuth()) return false;
  }

  // Get the output sample/line coordinate
  outSample = p_outmap->WorldX();
  outLine = p_outmap->WorldY();

  // Make sure the point is inside the output image
  if(outSample < 0.5) return false;
  if(outLine < 0.5) return false;
  if(outSample > p_outputSamples + 0.5) return false;
  if(outLine > p_outputLines + 0.5) return false;

  // Everything is good
  return true;
}

int ringscam2mapForward::OutputSamples() const {
  return p_outputSamples;
}

int ringscam2mapForward::OutputLines() const {
  return p_outputLines;
}


// Transform object constructor
ringscam2mapReverse::ringscam2mapReverse(const int inputSamples,
                                         const int inputLines, Camera *incam,
                                         const int outputSamples,
                                         const int outputLines,
                                         RingPlaneProjection *outmap, bool trim) {
                                         // Planar *outmap, bool trim) {

  p_inputSamples = inputSamples;
  p_inputLines = inputLines;
  p_incam = incam;

  p_outputSamples = outputSamples;
  p_outputLines = outputLines;
  p_outmap = outmap;

  p_trim = trim;
}

// Transform method mapping output line/samps to rads/azs to input line/samps
bool ringscam2mapReverse::Xform(double &inSample, double &inLine,
                           const double outSample, const double outLine) {
  // See if the output image coordinate converts to lat/lon
  if(!p_outmap->SetWorld(outSample, outLine)) return false;

  // See if we should trim
 if((p_trim) && (p_outmap->HasGroundRange())) {
   if(p_outmap->Radius() < p_outmap->MinimumRadius()) return false;
   if(p_outmap->Radius() > p_outmap->MaximumRadius()) return false;
   if(p_outmap->Azimuth() < p_outmap->MinimumAzimuth()) return false;
   if(p_outmap->Azimuth() > p_outmap->MaximumAzimuth()) return false;
 }

  // Get the universal rad/az and see if it can be converted to input line/samp
  double radius = p_outmap->Radius();
  double az = p_outmap->UniversalAzimuth();

  if(!p_incam->SetUniversalGround(radius, az)) return false;

  // Make sure the point is inside the input image
  if(p_incam->Sample() < 0.5) return false;
  if(p_incam->Line() < 0.5) return false;
  if(p_incam->Sample() > p_inputSamples + 0.5) return false;
  if(p_incam->Line() > p_inputLines + 0.5) return false;

  // Everything is good
  inSample = p_incam->Sample();
  inLine = p_incam->Line();

  return true;
}

int ringscam2mapReverse::OutputSamples() const {
  return p_outputSamples;
}

int ringscam2mapReverse::OutputLines() const {
  return p_outputLines;
}

void BandChange(const int band) {
  incam->SetBand(band);
}

// Helper function to print out mapfile to session log
void PrintMap() {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFileName("MAP"));

  PvlGroup &userGrp = userMap.FindGroup("Mapping", Pvl::Traverse);

  //Write map file out to the log
  Application::GuiLog(userGrp);
}

// Helper function to get mapping resolution.
void LoadMapRes() {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFileName("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping", Pvl::Traverse);

  // Set resolution
  if(userGrp.HasKeyword("Scale")) {
    ui.Clear("RESOLUTION");
    ui.PutDouble("RESOLUTION", userGrp["Scale"]);
    ui.Clear("PIXRES");
    ui.PutAsString("PIXRES", "PPD");
  }
  else if(userGrp.HasKeyword("PixelResolution")) {
    ui.Clear("RESOLUTION");
    ui.PutDouble("RESOLUTION", userGrp["PixelResolution"]);
    ui.Clear("PIXRES");
    ui.PutAsString("PIXRES", "MPP");
  }
  else {
    QString msg = "No resolution value found in [" + ui.GetFileName("MAP") + "]";
    throw IException(IException::User, msg, _FILEINFO_);
  }
}

//Helper function to get camera resolution.
void LoadCameraRes() {
  UserInterface &ui = Application::GetUserInterface();
  QString file = ui.GetFileName("FROM");

  // Open the input cube, get the camera object, and the cam map projection
  Cube c;
  c.open(file);
  Camera *cam = c.camera();
  Pvl camMap;
  cam->BasicMapping(camMap);
  PvlGroup &camGrp = camMap.FindGroup("Mapping");

  ui.Clear("RESOLUTION");
  ui.PutDouble("RESOLUTION", camGrp["PixelResolution"]);

  ui.Clear("PIXRES");
  ui.PutAsString("PIXRES", "MPP");
}

//Helper function to get ground range from map file.
void LoadMapRange() {
  UserInterface &ui = Application::GetUserInterface();

  // Get map file
  Pvl userMap;
  userMap.Read(ui.GetFileName("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping", Pvl::Traverse);

  // Set ground range keywords that are found in mapfile
  int count = 0;
  ui.Clear("MINRAD");
  ui.Clear("MAXRAD");
  ui.Clear("MINAZ");
  ui.Clear("MAXAZ");
  if(userGrp.HasKeyword("MinimumRadius")) {
    ui.PutDouble("MINRAD", userGrp["MinimumRadius"]);
    count++;
  }
  if(userGrp.HasKeyword("MaximumRadius")) {
    ui.PutDouble("MAXRAD", userGrp["MaximumRadius"]);
    count++;
  }
  if(userGrp.HasKeyword("MinimumAzimuth")) {
    ui.PutDouble("MINAZ", userGrp["MinimumAzimuth"]);
    count++;
  }
  if(userGrp.HasKeyword("MaximumAzimuth")) {
    ui.PutDouble("MAXAZ", userGrp["MaximumAzimuth"]);
    count++;
  }

  // Set default ground range param to map
  ui.Clear("DEFAULTRANGE");
  ui.PutAsString("DEFAULTRANGE", "MAP");

  if(count < 4) {
    QString msg = "One or more of the values for the ground range was not found";
    msg += " in [" + ui.GetFileName("MAP") + "]";
    throw IException(IException::User, msg, _FILEINFO_);
  }
}

//Helper function to load camera range.
void LoadCameraRange() {
  UserInterface &ui = Application::GetUserInterface();
  QString file = ui.GetFileName("FROM");

  // Get the map projection file provided by the user
  Pvl userMap;
  userMap.Read(ui.GetFileName("MAP"));

  // Open the input cube, get the camera object, and the cam map projection
  Cube c;
  c.open(file);
  Camera *cam = c.camera();

  // Make the target info match the user mapfile
  double minrad, maxrad, minaz, maxaz;
  cam->ringRange(minrad, maxrad, minaz, maxaz, userMap);

  // Set ground range parameters in UI
  ui.Clear("MINRAD");
  ui.PutDouble("MINRAD", minrad);
  ui.Clear("MAXRAD");
  ui.PutDouble("MAXRAD", maxrad);
  ui.Clear("MINAZ");
  ui.PutDouble("MINAZ", minaz);
  ui.Clear("MAXAZ");
  ui.PutDouble("MAXAZ", maxaz);

  // Set default ground range param to camera
  ui.Clear("DEFAULTRANGE");
  ui.PutAsString("DEFAULTRANGE", "CAMERA");
}

