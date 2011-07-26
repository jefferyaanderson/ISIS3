#ifndef Latitude_h
#define Latitude_h

/**
 * @file
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software
 *   and related material nor shall the fact of distribution constitute any such
 *   warranty, and no responsibility is assumed by the USGS in connection
 *   therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see
 *   the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include "Angle.h"

namespace Isis {
  class Distance;
  class PvlGroup;

  /**
   * This class is designed to encapsulate the concept of a Latitude. This is
   *   used primary for surface points but is also a general purpose class.
   *   This class has error checking for past the poles. This adds
   *   the concept of 90/-90 and planetographic to the Angle class.
   *
   * @ingroup Utility
   *
   * @author 2010-10-13 Steven Lambright
   *
   * @internal
   *   @history 2011-01-25 Steven Lambright - Added a constructor which takes a
   *                         mapping group.
   */
  class Latitude : public Angle {
    public:
      /**
       * Some user-configurable error checking parameters. This is meant to be
       *   used as a bit flag.
       *
       *  Example:
       *    Latitude(95, Angle::Degrees, Latitude::AllowPastPole)
       *    This will allow latitudes past 90 and not throw an exception.
       *
       *    Latitude(95, Angle::Degrees,
       *             Latitude::AllowPastPole | Latitude::ThrowAllErrors)
       *    This will allow latitudes past 90 still and not throw an exception.
       *
       *    Latitude(95, Angle::Degrees, Latitude::ThrowAllErrors)
       *    This will throw an exception.
       *
       *    Latitude(95, Angle::Degrees)
       *    This will throw an exception.
       */
      enum ErrorChecking {
        //! Throw an exception if any problems are found (default value).
        ThrowAllErrors = 0,
        //! Don't throw an exception if a latitude beyond -90/90 is found
        AllowPastPole
      };

      /**
       * These are the latitude coordinate systems. The universal system is
       *   Planetocentric and this class is heavily geared towards using them.
       *   If you wish to use Planetographic, planetary radii must be provided
       *   and at the moment latitudes past 90 aren't supported in
       *   Planetographic.
       */
      enum CoordinateType {
        /**
         * This is the universal (and default) latitude coordinate system.
         * Latitudes in this system are the angle from the the equatorial plane
         * (the line at 0 degrees latitude) to the requested latitude.
         */
        Planetocentric,
        /**
         * This is a secondary coordinate system for latitudes.
         *
         * Latitudes in this system are angle between the equatorial plane and
         *   the perpendicular to the surface at the point. Conceptually you
         *   draw the perpendicular to the surface back to the equatorial plane
         *   and the angle created is the planetographic latitude. This doesn't
         *   differ from Planetocentric on a sphere. These latitudes require the
         *   planetary radii to work with them at all, since the radii affect
         *   the latitude values themselves.
         */
        Planetographic
      };

      Latitude();
      Latitude(double latitude,
               Angle::Units latitudeUnits,
               ErrorChecking errors = ThrowAllErrors);

      Latitude(Angle latitude, ErrorChecking errors = ThrowAllErrors);

      Latitude(Angle latitude,
               PvlGroup mapping,
               ErrorChecking errors = ThrowAllErrors);

      Latitude(double latitude,
               PvlGroup mapping,
               Angle::Units latitudeUnits,
               ErrorChecking errors = ThrowAllErrors);

      Latitude(double latitude, Distance equatorialRadius, Distance polarRadius,
               CoordinateType latType = Planetocentric,
               Angle::Units latitudeUnits = Angle::Radians,
               ErrorChecking errors = ThrowAllErrors);

      Latitude(const Latitude &latitudeToCopy);

      ~Latitude();

      double GetPlanetocentric(Angle::Units units = Angle::Radians) const;
      void SetPlanetocentric(double latitude,
                             Angle::Units units = Angle::Radians);

      double GetPlanetographic(Angle::Units units = Angle::Radians) const;
      void SetPlanetographic(double latitude,
                           Angle::Units units = Angle::Radians);

      bool IsInRange(Latitude min, Latitude max) const;

      Latitude& operator=(const Latitude & latitudeToCopy);
    
      /**
       * Same as GetPlanetocentric.
       *
       * @see GetPlanetocentric
       
      operator double() const {
        return GetPlanetocentric();
      }*/

    protected:
      virtual void SetAngle(double angle, const Angle::Units &units);

    private:
      /**
       * Used for converting to Planetographic, this is the radius of the target
       *   on the equatorial plane.
       */
      Distance *p_equatorialRadius;
      /**
       * Used for converting to Planetographic, this is the radius of the target
       *   perpendicular to the equatorial plane.
       */
      Distance *p_polarRadius;
      
      //! This contains which exceptions should not be thrown
      ErrorChecking p_errors;
  };
}

#endif
