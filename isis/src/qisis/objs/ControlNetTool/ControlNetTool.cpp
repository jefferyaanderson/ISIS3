#include "ControlNetTool.h"

#include <sstream>
#include <vector>
#include <iomanip>

#include <QBrush>
#include <QDebug>
#include <QList>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QString>

#include "Application.h"
#include "ControlList.h"
#include "ControlMeasure.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlPointEditView.h"
#include "ControlPointEditWidget.h"
#include "CubeDnView.h"
#include "Directory.h"
#include "IException.h"
#include "MdiCubeViewport.h"
#include "Project.h"
#include "SerialNumber.h"
#include "ToolPad.h"
#include "UniversalGroundMap.h"
#include "ViewportMainWindow.h"
#include "Workspace.h"

using namespace std;

namespace Isis {
  /**
   * ControlNet (Qnet) tool - Handles mouse button actions and drawing control points on viewports
   *
   * @param parent Pointer to the parent widget for the ControlNet tool
   *
   * @author 2016-09-01 Tracie Sucharski
   *
   * @internal
   *
   */
  ControlNetTool::ControlNetTool (Directory *directory, QWidget *parent) : Tool(parent) {

    m_directory = directory;
    m_view = qobject_cast<CubeDnView *>(parent);
  }


  ControlNetTool::~ControlNetTool () {
  }


  /**
    * Adds the ControlNet tool action to the tool pad.
    *
    * @param pad Tool pad
    * @return @b QAction* Pointer to ControlNet tool action
    *
    * @internal
    *   @history 2017-07-25 Tyler Wilson - Set the
    *       object name for the QAction in this
    *       method.  It provides a convenient key
    *       to search through a list of actions in
    *       other classes.  References #4994.
    *
    */
   QAction *ControlNetTool::toolPadAction(ToolPad *pad) {
     QAction *action = new QAction(this);
     action->setIcon(QPixmap(toolIconDir()+"/HILLBLU_molecola.png"));
     action->setToolTip("Control Point Editor (T)");
     action->setStatusTip("If tool disabled, make sure you have a control net in your project and "
                          "it is set to the active control.");
     action->setShortcut(Qt::Key_T);

     //The object name is being set and used as a key to search with for this action in
     //other classes (ie. CubeDnView)
     action->setObjectName("ControlNetTool");

     QList<ControlList *> cnets = m_directory->project()->controls();

     //Check to see if there are any control nets within the project.
     //If not, then this action should be disabled.
     if (cnets.isEmpty()) {
       action->setDisabled(true);
     }

     return action;
   }


   /**
    * Set the active control net to be used for editing
    *
    * @param cnet (ControlNet *)  The active control net from Directory that is being used for
    *             editing
    */
   void ControlNetTool::setControlNet(ControlNet *cnet) {
     m_controlNet = cnet;
     qDebug()<<"ControlNetTool::setControlNet";
     //  Cannot use Tool::cubeViewportList() because it does not properly return a NULL if viewports
     //  don't exist.
     if (workspace() && workspace()->cubeViewportList()) {
       qDebug()<<"ControlNetTool::setControlNet  before paintAllViewports";
       paintAllViewports();
     }
   }


   void ControlNetTool::loadNetwork() {
     qDebug()<<"ControlNetTool::loadNetwork control";
     setControlNet(m_directory->project()->activeControl()->controlNet());
   }


  /**
   * Handle mouse events on CubeViewport
   *
   * @param p[in]   (QPoint)            Point under cursor in cubeviewport
   * @param s[in]   (Qt::MouseButton)   Which mouse button was pressed
   *
   * @author 2016-09-01 Tracie Sucharski
   *
   * @internal
   */
  void ControlNetTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    MdiCubeViewport *cvp = cubeViewport();
    if (m_controlNet == NULL || cvp  == NULL) return;

    // Determine if the cvp is a Shape
    //  Get all ShapeLists from Project
    bool isGroundSource = m_view->viewportContainsShape(cvp);

    double samp,line;
    cvp->viewportToCube(p.x(),p.y(),samp,line);
    QString sn = SerialNumber::Compose(cvp->cube()->fileName());

    if (s == Qt::LeftButton) {

//      if (sn == m_groundSN) {
//        QString message = "Cannot select point for editing on ground source.  Select ";
//        message += "point using un-projected images or the Navigator Window.";
//        QMessageBox::critical(m_ControlNetTool, "Error", message);
//        return;
//      }

      //  Find closest control point in network
      // since we are in a connected slot, we need to handle exceptions thrown by FindClosest
      try {
        ControlPoint *point = m_controlNet->FindClosest(sn, samp, line);
        emit modifyControlPoint(point, sn);
      }
      catch (IException &ie) {
        QString message = "No points exist for editing. Create points using the right mouse";
        message += " button.";
        QMessageBox::warning(m_ControlNetTool, "Warning", message);
        return;
      }
    }

    else if (s == Qt::MidButton) {

      if (!m_controlNet || m_controlNet->GetNumPoints() == 0) {
        QString message = "No points exist for deleting.  Create points ";
        message += "using the right mouse button.";
        QMessageBox::warning(m_ControlNetTool, "Warning", message);
        return;
      }

//      if (m_groundOpen && file == m_groundCube->fileName()) {
//        QString message = "Cannot select point for deleting on ground source.  Select ";
//        message += "point using un-projected images or the Navigator Window.";
//        QMessageBox::critical(m_ControlNetTool, "Error", message);
//        return;
//      }

      //  Find closest control point in network
      ControlPoint *point = NULL;
      try {
        point = m_controlNet->FindClosest(sn, samp, line);

        if (point == NULL) {
          QString message = "No points exist for deleting.  Create points ";
          message += "using the right mouse button.";
          QMessageBox::warning(m_ControlNetTool, "Warning", message);
          return;
        }
      }
      catch (IException &e) {
        QString message = "Cannot find point on this image for deleting.";
        QMessageBox::critical(m_ControlNetTool, "Error", message);
        return;
      }

      emit deleteControlPoint(point);
    }
    else if (s == Qt::RightButton) {

      UniversalGroundMap *gmap = cvp->universalGroundMap();
      if (!gmap->SetImage(samp,line)) {
        QString message = "Invalid latitude or longitude at this point. ";
        QMessageBox::critical(NULL, "Error", message);
        return;
      }
      double lat = gmap->UniversalLatitude();
      double lon = gmap->UniversalLongitude();
      emit createControlPoint(lat, lon, cvp->cube(), isGroundSource);
    }
  }


  /**
   * This will draw the control measures on the given cube viewport
   *
   * @param vp (MdiCubeViewport *) Cube viewport control measures are drawn on
   * @param painter (QPainter)  The painter used for the drawing
   */
  void ControlNetTool::paintViewport(MdiCubeViewport *vp, QPainter *painter) {

    if (m_controlNet) {
      drawAllMeasurements(vp, painter);
    }
  }


  /**
   * This method will repaint the Control Points in each viewport
   *
   * @internal
   * @history 2010-06-03 Jeannie Walldren - Removed "std::" since "using
   *                          namespace std"
   */
  void ControlNetTool::paintAllViewports() {

    // Take care of drawing things on all viewPorts.
    // Calling update will cause the Tool class to call all registered tools
    // if point has been deleted, this will remove it from the main window
    if (cubeViewportList()) {
      MdiCubeViewport *vp;
      for (int i=0; i<(int)cubeViewportList()->size(); i++) {
        vp = (*(cubeViewportList()))[i];
         vp->viewport()->update();
      }
    }
  }


  /**
   * Draw all measurments located on the image in this viewPort
   *
   * @param vp (MdiCubeViewport *) Viewport where measurements will be drawn
   * @param painter (QPainter *) Does the actual drawing on the viewport widget
   *
   * @internal
   *   @history 2010-06-03 Jeannie Walldren - Removed "std::" since "using
   *                          namespace std"
   *   @history 2010-06-08 Jeannie Walldren - Fixed bug that was causing ignored
   *                          measures not be drawn as yellow unless ControlNetTool was
   *                          open
   *   @history 2010-07-01 Jeannie Walldren - Modified to draw points selected in
   *                          ControlNetTool last so they lay on top of all other points
   *                          in the image.
   *   @history 2011-04-15 Tracie Sucharski - Fixed bug which was causing all
   *                          measures to be drawn on all cubes.  Also removed
   *                          loop through measures, instead just get measure
   *                          for given serial number.
   *   @history 2011-10-20 Tracie Sucharski - Add check for a control network
   *                          that does not yet have any control points.
   *   @history 2011-11-09 Tracie Sucharski - If there are no measures for
   *                          this cube, return.
   *   @history 2017-07-31 Tracie Sucharski - The current edit point will be drawn in red as a
   *                          crosshair enclosed by circle.
   */
  void ControlNetTool::drawAllMeasurements(MdiCubeViewport *vp, QPainter *painter) {

    // Without a controlnetwork there are no points, or if new net, no points
    if (m_controlNet == 0 || m_controlNet->GetNumPoints() == 0) return;

    // Don't show the measurments on cubes not in the serial number list
    // TODO: Should we show them anyway
    // TODO: Should we add the SN to the viewPort
    QString serialNumber = SerialNumber::Compose(*vp->cube(), true);

//    if (serialNumber == m_groundSN) {
//      drawGroundMeasures(vp, painter);
//      return;
//    }

    if (!m_controlNet->GetCubeSerials().contains(
                      serialNumber)) return;

    QList<ControlMeasure *> measures =
        m_controlNet->GetMeasuresInCube(serialNumber);
    // loop through all measures contained in this cube
    for (int i = 0; i < measures.count(); i++) {
      ControlMeasure *m = measures[i];
      // Find the measurments on the viewport
      double samp = m->GetSample();
      double line = m->GetLine();
      int x, y;
      vp->cubeToViewport(samp, line, x, y);
      // if the point is ignored,
      if (m->Parent()->IsIgnored()) {
        painter->setPen(QColor(255, 255, 0)); // set point marker yellow
      }
      // point is not ignored, but measure matching this image is ignored,
      else if (m->IsIgnored()) {
        painter->setPen(QColor(255, 255, 0)); // set point marker yellow
      }
      // Neither point nor measure is not ignored and the measure is fixed,
      else if (m->Parent()->GetType() != ControlPoint::Free) {
        painter->setPen(Qt::magenta);// set point marker magenta
      }
      else {
        painter->setPen(Qt::green); // set all other point markers green
      }
      // draw points
      painter->drawLine(x - 5, y, x + 5, y);
      painter->drawLine(x, y - 5, x, y + 5);
    }
    // if ControlNetTool is open, the editPointId will contain the ControlPoint Id of the current edit
    // point.
    ControlPoint *currentEditPoint = NULL;
    if (m_directory->controlPointEditView()) {
      currentEditPoint = m_directory->controlPointEditView()->controlPointEditWidget()->editPoint();
    }
    if (currentEditPoint && m_controlNet->ContainsPoint(currentEditPoint->GetId())) {
      // Selected point is in the image,
      if (currentEditPoint->HasSerialNumber(serialNumber)) {
        // find the measurement
        double samp = (*currentEditPoint)[serialNumber]->GetSample();
        double line = (*currentEditPoint)[serialNumber]->GetLine();
        int x, y;
        vp->cubeToViewport(samp, line, x, y);

        QPainterPath path;
        //  Draw circle, then crosshair inside circle
        path.addEllipse(QPointF(x,y), 5., 5.);
        path.moveTo(x, y-5);
        path.lineTo(x, y+5);
        path.moveTo(x-5, y);
        path.lineTo(x+5, y);

//        QBrush brush(Qt::red);
        QPen pen(QPen(Qt::red, 0.0));
        // draw the selected point in each image last so it's on top of the rest of the points
        painter->setPen(pen);
        painter->drawPath(path);
      }
    }
  }
}
