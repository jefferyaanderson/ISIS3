#ifndef GisTopology_h
#define GisTopology_h
/**                                                                       
 * @file                                                                  
 * $Revision: 6163 $ 
 * $Date: 2015-05-01 18:04:34 -0700 (Fri, 01 May 2015) $ 
 * $Id: GisTopology.h 6163 2015-05-02 01:04:34Z kbecker@GS.DOI.NET $
 *                                                                        
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


// geos library for types GEOSGeometry, GEOSPreparedGeometry, GEOSCoordSequence, 
// GEOSWKTReader, GEOSWKTWriter, GEOSWKBReader, GEOSWKBWriter 
#include <geos_c.h>

class QString;

namespace Isis {

  /** 
   * This class models GIS topology. This class allows us to create GEOS 
   * geometries from well-known binary (WKB) strings or well-known text (WKT) 
   * strings. It also allows us to create WKB or WKT strings from a GEOS 
   * geometry. 
   *  
   * @author 2012-07-15 Kris Becker
   * @internal 
   *   @history 2012-07-15 Kris Becker - Original version.
   *   @history 2015-03-18 Jeannie Backer - Brought class files closer to ISIS coding standards.
   *   @history 2015-03-23 Jeannie Backer - Improved documentation.
   *   @history 2015-04-22 Jeannie Backer - Fixed typo "wkb" where it should have been "wkt".
   *   @history 2016-03-02 Ian Humphrey - Updated for coding standards compliance, fixed minor
   *                           documentation issues, and added to jwbacker's unit test in
   *                           preparation for adding this class to ISIS. Fixes #2398.
   */
  class GisTopology {
    public:

      /**
       * Enumeration to indicate whether the geometry should be preserved.
       */
      enum Disposition { 
             DestroyGeometry, //!< Destroy the geometry.
             PreserveGeometry //!< Preserve the geometry.
      };
  
      static GisTopology *instance();
  
      GEOSGeometry *geomFromWKB(const QString &wkb);
      GEOSGeometry *geomFromWKT(const QString &wkt);
      GEOSGeometry *clone(const GEOSGeometry *geom) const;
      const GEOSPreparedGeometry *preparedGeometry(const GEOSGeometry *geom) const;
  
  
      QString wkb(const GEOSGeometry *geom, 
                  const Disposition &disp = PreserveGeometry);
      QString wkt(const GEOSGeometry *geom, 
                  const Disposition &disp = PreserveGeometry);
  
      void destroy(GEOSGeometry *geom) const;
      void destroy(const GEOSGeometry *geom) const;
      void destroy(const GEOSPreparedGeometry *ppgeom) const;
      void destroy(GEOSCoordSequence *sequence) const;
      void destroy(const unsigned char *geos_text) const;
      void destroy(const char *geos_text) const;
  
    private:
      GisTopology();
      ~GisTopology();
  
      static void dieAtExit();
  
      void initialize();
  
      //  Initialize and shut down GEOS C API
      void geosInit();
      void geosFinish();
  
      static void notice(const char *fmt, ...);
      static void error(const char *fmt, ...);
       
      //  Reader/Writer allocations
      GEOSWKTReader *wktReader();
      GEOSWKTWriter *wktWriter();
  
      GEOSWKBReader *wkbReader();
      GEOSWKBWriter *wkbWriter();
  
      static GisTopology *m_gisfactory; //!< A static member variable representing the GIS factory.
      GEOSWKTReader *m_WKTreader;       //!< A GEOS library parser for well-known text format.
      GEOSWKTWriter *m_WKTwriter;       //!< A GEOS library writer for well-known text format.
      GEOSWKBReader *m_WKBreader;       //!< A GEOS library parser for well-known binary format.
      GEOSWKBWriter *m_WKBwriter;       //!< A GEOS library writer for well-known binary format. 
  
  };

}  // namespace Isis

#endif


