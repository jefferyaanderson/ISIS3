#ifndef BundleSettings_h
#define BundleSettings_h

/**
 * @file
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include <QList>
#include <QPair>

#include "MaximumLikelihoodWFunctions.h" // why not forward declare???

namespace Isis {
  class MaximumLikelihoodWFunctions;
  class PvlGroup;
  /**
   * @brief Container class for BundleAdjustment settings. 
   * This class contains all of the settings needed to run a bundle adjustment. 
   * A BundleSettings object is passed into the BundleAdjustment constructor.  
   *  
   * @ingroup ControlNetworks
   *
   * @author 2014-05-14 Jeannie Backer
   *
   * @internal
   *   @history 2014-05-14 Jeannie Backer - Original version.
   *  
   */
 class BundleSettings {
   public:
      BundleSettings();
      ~BundleSettings();

      void setValidateNetwork(bool validate);
      bool validateNetwork();

      // Solve Options
      /**
       * This enum defines the types of solve methods.
       */
      enum SolveMethod {
        Sparse,   //!< CholMod
        SpecialK, //!< Dense
        OldSparse //!< Sparse Matrix Solver
      };
      // converter
      static SolveMethod stringToSolveMethod(QString solveMethod);
      static QString solveMethodToString(SolveMethod solveMethod);
      // mutators
      void setSolveOptions(SolveMethod method, 
                           bool solveObservationMode = false,
                           bool updateCubeLabel = false, 
                           bool errorPropagation = false,
                           bool solveRadius = false, 
                           double globalLatitudeAprioriSigma = -1.0, 
                           double globalLongitudeAprioriSigma = -1.0, 
                           double globalRadiusAprioriSigma = -1.0);
//      void setSolveOptions(SolveMethod method, 
//                           bool updateCubeLabel = false,
//                           bool errorPropagation = false, 
//                           bool solveRadius = false, 
//                           double globalLatitudeAprioriSigma = -1.0, 
//                           double globalLongitudeAprioriSigma = -1.0, 
//                           double globalRadiusAprioriSigma = -1.0);
      void setSolveMethod(SolveMethod method);
      void setSolveObservationMode(bool observationMode);
      void setSolveRadius(bool radius);
      void setUpdateCubeLabel(bool updateCubeLabel);
      void setErrorPropagation(bool errorPropagation);
      void setOutlierRejection(bool outlierRejection, double multiplier);
      // accessors
      SolveMethod solveMethod() const;
      bool solveObservationMode() const;
      bool solveRadius() const;
      bool updateCubeLabel() const;
      bool errorPropagation() const;
      bool outlierRejection() const;
      double outlierRejectionMultiplier() const;
      double globalLatitudeAprioriSigma() const;
      double globalLongitudeAprioriSigma() const;
      double globalRadiusAprioriSigma() const;

      // Spacecraft Position Options
      /**
       * This enum defines the options for solving for observer position information (SPK).
       */
      enum ObserverPositionSolveOption {
        NoPositionFactors,            /**< Solve for none of the position factors.*/
        PositionOnly,                 /**< Solve for observer positions only.*/
        PositionVelocity,             /**< Solve for observer positions and velocities.*/
        PositionVelocityAcceleration, /**< Solve for observer positions, velocities, and  
                                           accerlations.*/
        AllPositionCoefficients       /**< Solve for all coefficients in the polynomials fit to
                                           the observer positions.*/
      };
      static ObserverPositionSolveOption stringToObserverPositionSolveOption(QString option);
      static QString observerPositionSolveOptionToString(ObserverPositionSolveOption option);
//      void setObserverPositionSolveOptions(ObserverPositionSolveOption option,
//                                           bool solveOverHermiteSpline = false,
//                                           int spkDegree = 2, int spkSolveDegree = 2);
      void setObserverPositionSolveOptions(ObserverPositionSolveOption option,
                           bool solveOverHermiteSpline = false,
                           int spkDegree = 2, int spkSolveDegree = 2,
                           double globalObserverPositionAprioriSigma = -1.0,  
                           double globalObserverVelocityAprioriSigma = -1.0,  
                           double globalObserverAccelerationAprioriSigma = -1.0);
      ObserverPositionSolveOption observerPositionSolveOption() const;
      bool solveObserverPositionOverHermiteSpline() const;
      int spkDegree() const;
      int spkSolveDegree() const;
      double globalObserverPositionAprioriSigma() const;
      double globalObserverVelocityAprioriSigma() const;
      double globalObserverAccelerationAprioriSigma() const;

      // Camera Pointing Options
      /**
       * This enum defines the options for solving for observer orientation information (CK). 
       */
      enum ObserverOrientationSolveOption {
        NoOrientationFactors,       /**< Solve for none of the orientation factors.*/
        AnglesOnly,                 /**< Solve for orientation angles: right ascension, declination
                                         and, optionally, twist.*/
        AnglesVelocity,             //!< Solve for orientation angles and their angular velocities.
        AnglesVelocityAcceleration, /**< Solve for orientation angles, their velocities and their 
                                         accelerations.*/
        AllOrientationCoefficients  /**< Solve for all coefficients in the polynomials fit to the
                                         orientation angles.*/
      };
      static ObserverOrientationSolveOption stringToObserverOrientationSolveOption(QString option);
      static QString observerOrientationSolveOptionToString(ObserverOrientationSolveOption option);
//      void setObserverOrientationSolveOptions(ObserverOrientationSolveOption option, 
//                                              bool solveTwist,  bool fitOverExisting = false,
//                                              int ckDegree = 2, int ckSolveDegree = 2);
      void setObserverOrientationSolveOptions(ObserverOrientationSolveOption option,
                           bool solveTwist,  bool fitOverExisting = false,
                           int ckDegree = 2, int ckSolveDegree = 2,
                           double globalObserverOrientationAnglesAprioriSigma = -1.0,
                           double globalObserverOrientationAngularVelocityAprioriSigma = -1.0,
                           double globalObserverOrientationAngularAccelerationAprioriSigma = -1.0);
      ObserverOrientationSolveOption observerOrientationSolveOption() const;
      bool solveTwist() const;
      bool fitOrientationPolynomialOverExisting() const;
      int ckDegree() const;
      int ckSolveDegree() const;
      double globalObserverOrientationAnglesAprioriSigma() const;
      double globalObserverOrientationAngularVelocityAprioriSigma() const;
      double globalObserverOrientationAngularAccelerationAprioriSigma() const;

      // Convergence Criteria
      /**
       * This enum defines the options for convergence. 
       */
      enum ConvergenceCriteria {
        Sigma0,              /**< Sigma0 will be used to determine that the bundle adjustment has 
                                  converged.*/
        ParameterCorrections /**< All parameter corrections will be used to determine that the 
                                  bundle adjustment has converged.*/
      };
      static ConvergenceCriteria stringToConvergenceCriteria(QString criteria);
      static QString convergenceCriteriaToString(ConvergenceCriteria criteria);
      void setConvergenceCriteria(ConvergenceCriteria criteria, double threshold, 
                                  int maximumIterations);
      ConvergenceCriteria convergenceCriteria() const;
      double convergenceCriteriaThreshold() const;
      int convergenceCriteriaMaximumIterations() const;

      // Parameter Uncertainties (Weighting)
      // mutators
//      void setGlobalLatitudeAprioriSigma(double sigma);
//      void setGlobalLongitudeAprioriSigma(double sigma);
//      void setGlobalRadiiAprioriSigma(double sigma);
//      void setGlobalObserverPositionAprioriSigma(double sigma);
//      void setGlobalObserverVelocityAprioriSigma(double sigma);
//      void setGlobalObserverAccelerationAprioriSigma(double sigma);
//      void setGlobalObserverOrientationAnglesAprioriSigma(double sigma);
//      void setGlobalObserverOrientationAngularVelocityAprioriSigma(double sigma);
//      void setGlobalObserverOrientationAngularAccelerationAprioriSigma(double sigma);

      // Maximum Likelihood Estimation Options
      /**
       * This enum defines the options for maximum likelihood estimation. 
       */
      enum MaximumLikelihoodModel {
        NoMaximumLikelihoodEstimator, /**< Do not use a maximum likelihood model.*/
        Huber,                        /**< Use a Huber maximum likelihood model. This model
                                           approximates the L2 norm near zero and the L1 norm
                                           thereafter. This model has one continuous derivative.*/
        ModifiedHuber,                /**< Use a modified Huber maximum likelihood model. This model
                                           approximates the L2 norm near zero and the L1 norm
                                           thereafter. This model has two continuous derivative.*/
        Welsch,                       /**< Use a Welsch maximum likelihood model. This model
                                           approximates the L2 norm near zero, but then decays
                                           exponentially to zero.*/
        Chen                          /**< Use a Chen maximum likelihood model. This is a highly
                                           aggressive model that intentionally removes the largest
                                           few percent of residuals.???? */
      };
      static MaximumLikelihoodWFunctions::Model stringToMaximumLikelihoodModel(QString model);
      static QString maximumLikelihoodModelToString(MaximumLikelihoodWFunctions::Model model);
      void addMaximumLikelihoodEstimatorModel(MaximumLikelihoodWFunctions::Model model, 
                                              double cQuantile);
      QList< QPair< MaximumLikelihoodWFunctions::Model, double > > 
          maximumLikelihoodEstimatorModels() const;

      // Self Calibration ??? (from cnetsuite only)

      // Target Body ??? (from cnetsuite only)

      // Output Options ??? (from Jigsaw only)
      void setOutputFiles(QString outputFilePrefix, bool createBundleOutputFile, 
                          bool createCSVPointsFile, bool createResidualsFile);
      QString outputFilePrefix() const;
      bool createBundleOutputFile() const;
      bool createCSVPointsFile() const;
      bool createResidualsFile() const;
      PvlGroup pvlGroup() const;

    private:
      bool m_validateNetwork;
      bool m_solveObservationMode;
      bool m_solveRadius;
      bool m_updateCubeLabel;
      bool m_errorPropagation;
      bool m_outlierRejection;
      double m_outlierRejectionMultiplier; // multiplier = 1 if rejection = false
      SolveMethod m_solveMethod;

      // Spacecraft Position Options
      ObserverPositionSolveOption m_observerPositionSolveOption;
      bool m_solvePositionOverHermiteSpline; // false if option = none
      int m_spkDegree; // not used unless option = all
      int m_spkSolveDegree; // not used unless option = all

      // Camera Pointing Options
      ObserverOrientationSolveOption m_observerOrientationSolveOption;
      bool m_solveTwist; // false if option = none
      bool m_fitOrientationPolynomialOverExisting; // false if option = none
      int m_ckDegree; // not used unless option = all
      int m_ckSolveDegree; // not used unless option = all

      // Convergence Criteria
      ConvergenceCriteria m_convergenceCriteria;
      double m_convergenceCriteriaThreshold;
      int m_convergenceCriteriaMaximumIterations;

      // Parameter Uncertainties (Weighting)
      double m_globalLatitudeAprioriSigma;
      double m_globalLongitudeAprioriSigma;
      double m_globalRadiusAprioriSigma;
      double m_globalObserverPositionAprioriSigma;
      double m_globalObserverVelocityAprioriSigma;
      double m_globalObserverAccelerationAprioriSigma;
      double m_globalObserverOrientationAnglesAprioriSigma;
      double m_globalObserverOrientationVelocityAprioriSigma;
      double m_globalObserverOrientationAccelerationAprioriSigma;

      // Maximum Likelihood Estimation Options
      // model and maxModelCQuantile for each
      // of the three maximum likelihood
      // estimations.  Note that Welsch and
      // Chen can not be used for the first
      // model.
      QList< QPair< MaximumLikelihoodWFunctions::Model, double > > m_maximumLikelihood;

      // Self Calibration ??? (from cnetsuite only)

      // Target Body ??? (from cnetsuite only)

      // Output Options ??? (from Jigsaw only)
      QString m_outputFilePrefix;
      bool m_createBundleOutputFile;
      bool m_createCSVPointsFile;
      bool m_createResidualsFile;
 };
};
#endif