// author: Zenan-SSR
// date: 2022/10/11
// descrition: Using esp32 uart control AUBO movement

#include "TENG_experiment.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include "util.h"


#define SERVER_HOST "192.168.100.120"
//#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8899


// Constructor function
TENG_experiment::TENG_experiment()
{
    int ret = aubo_robot_namespace::InterfaceCallSuccCode;

    //Login
    ret = TENG_experiment::robotService.robotServiceLogin(SERVER_HOST, SERVER_PORT, "aubo", "123456");
    if(ret == aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cout<<"login successful."<<std::endl;
    }
    else
    {
        std::cerr<<"login failed."<<std::endl;
    }

    //Initialize motion properties
    TENG_experiment::robotService.robotServiceInitGlobalMoveProfile();

    //Set the maximum acceleration of articulated motion
    aubo_robot_namespace::JointVelcAccParam jointMaxAcc;
    aubo_robot_namespace::JointVelcAccParam jointMaxVelc;
    for(int i=0;i<6;i++)
    {
        jointMaxAcc.jointPara[i]  = 20.0/180.0*M_PI;   //The unit is radians
        jointMaxVelc.jointPara[i] = 20.0/180.0*M_PI;   //The unit is radians
    }
    TENG_experiment::robotService.robotServiceSetGlobalMoveJointMaxAcc(jointMaxAcc);
    TENG_experiment::robotService.robotServiceSetGlobalMoveJointMaxVelc(jointMaxVelc);


    //Set the maximum acceleration of the end motion. Linear motion is the end motion.
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineAcc(2);    //Units m/s2
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineVelc(2);   //Units m/s


    // get current joint state
    ret = TENG_experiment::robotService.robotServiceGetRobotJointStatus(TENG_experiment::jointStatus, 6);
    if(ret == aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cout<<"Get joint status successfully."<<std::endl;
        Util::printJointStatus(TENG_experiment::jointStatus, 6);
    }
    else
    {
        std::cerr<<"Failed to get joint state."<<std::endl;
    }
    sleep(1);

    // update current joint state
    for(int i = 0;i < 6;i++)
    {
        TENG_experiment::jointAngle[i] = TENG_experiment::jointStatus[i].jointPosJ;
    }

    //Preparation point
//    Util::initJointAngleArray(TENG_experiment::jointAngle, -0.9337, -0.0810, -2.0355, -0.3928, -1.6103, -2.6021); // 初始位置状态
    ret = TENG_experiment::robotService.robotServiceJointMove(TENG_experiment::jointAngle, true);   //Joint movement to preparation point
    if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cerr<<"Joint move failed.　ret:"<<ret<<std::endl;
    }
    else
    {
        std::cout<<"Preparation point done!"<<std::endl;
    }
}

// Deconstructor function
TENG_experiment::~TENG_experiment()
{
    /** Robotic arm shutdown **/
    TENG_experiment::robotService.robotServiceRobotShutdown();

    /** Interface call: logout　**/
    TENG_experiment::robotService.robotServiceLogout();
}


void TENG_experiment::vertical_movement()
{
    for(int i=0;i<100;i++)
    {
//        Util::initJointAngleArray(TENG_experiment::jointAngle, -0.9337, -0.0810, -2.0355, -0.3928, -1.6103, -2.6021); // 初始位置状态
        int ret = TENG_experiment::robotService.robotServiceLineMove(TENG_experiment::jointAngle, true);
        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
        {
            std::cerr<<"Move failed.　ret:"<<ret<<std::endl;
            exit(1);
        }
        else
        {
            std::cout<<"This is the "<<i<<" Movement."<<std::endl;
        }

        //Linear motion
        aubo_robot_namespace::MoveRelative relativeMoveOnBase;
        relativeMoveOnBase.ena = true;
        relativeMoveOnBase.relativePosition[0] = 0;
        relativeMoveOnBase.relativePosition[1] = 0;
        relativeMoveOnBase.relativePosition[2] = 0.05;   //Units m
        relativeMoveOnBase.relativeOri.w=1;
        relativeMoveOnBase.relativeOri.x=0;
        relativeMoveOnBase.relativeOri.y=0;
        relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(relativeMoveOnBase);

        aubo_robot_namespace::wayPoint_S centerOfMindWayPoint;
        TENG_experiment::robotService.robotServiceGetCurrentWaypointInfo(centerOfMindWayPoint);
        ret = TENG_experiment::robotService.robotServiceLineMove(centerOfMindWayPoint.jointpos, true);
        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
        {
            std::cerr<<"Move failed.　ret:"<<ret<<std::endl;
            exit(1);
        }
    }
}


void TENG_experiment::target_movement(int cmd, double step)
{
    // cmd run
    if(cmd == -1)
    {
        std::cerr<<"Wrong cmd! Move failed."<<std::endl;
        exit(1);
    }
    else if(cmd == 0) // 0 degree
    {
        //Linear motion
        TENG_experiment::relativeMoveOnBase.ena = true;
        TENG_experiment::relativeMoveOnBase.relativePosition[0] = step;
        TENG_experiment::relativeMoveOnBase.relativePosition[1] = 0;
        TENG_experiment::relativeMoveOnBase.relativePosition[2] = 0;   //Units m
        TENG_experiment::relativeMoveOnBase.relativeOri.w=1;
        TENG_experiment::relativeMoveOnBase.relativeOri.x=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.y=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(TENG_experiment::relativeMoveOnBase);
    }
    else if(cmd == 1) // 60 degree
    {
        //Linear motion
        TENG_experiment::relativeMoveOnBase.ena = true;
        TENG_experiment::relativeMoveOnBase.relativePosition[0] = -0.500 * step;
        TENG_experiment::relativeMoveOnBase.relativePosition[1] = -0.866 * step;
        TENG_experiment::relativeMoveOnBase.relativePosition[2] = 0;   //Units m
        TENG_experiment::relativeMoveOnBase.relativeOri.w=1;
        TENG_experiment::relativeMoveOnBase.relativeOri.x=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.y=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(TENG_experiment::relativeMoveOnBase);
    }
    else if(cmd == 2) // 120 degree
    {
        //Linear motion
        TENG_experiment::relativeMoveOnBase.ena = true;
        TENG_experiment::relativeMoveOnBase.relativePosition[0] = -0.500 * step;
        TENG_experiment::relativeMoveOnBase.relativePosition[1] = 0.866 * step;
        TENG_experiment::relativeMoveOnBase.relativePosition[2] = 0;   //Units m
        TENG_experiment::relativeMoveOnBase.relativeOri.w=1;
        TENG_experiment::relativeMoveOnBase.relativeOri.x=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.y=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(TENG_experiment::relativeMoveOnBase);
    }
    else if(cmd == 3) // -z
    {
        //Linear motion
        TENG_experiment::relativeMoveOnBase.ena = true;
        TENG_experiment::relativeMoveOnBase.relativePosition[0] = 0;
        TENG_experiment::relativeMoveOnBase.relativePosition[1] = 0;
        TENG_experiment::relativeMoveOnBase.relativePosition[2] = -1 * step;   //Units m
        TENG_experiment::relativeMoveOnBase.relativeOri.w=1;
        TENG_experiment::relativeMoveOnBase.relativeOri.x=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.y=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(TENG_experiment::relativeMoveOnBase);
    }
    else if(cmd == 4) // +z
    {
        //Linear motion
        TENG_experiment::relativeMoveOnBase.ena = true;
        TENG_experiment::relativeMoveOnBase.relativePosition[0] = 0;
        TENG_experiment::relativeMoveOnBase.relativePosition[1] = 0;
        TENG_experiment::relativeMoveOnBase.relativePosition[2] = step;   //Units m
        TENG_experiment::relativeMoveOnBase.relativeOri.w=1;
        TENG_experiment::relativeMoveOnBase.relativeOri.x=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.y=0;
        TENG_experiment::relativeMoveOnBase.relativeOri.z=0;
        TENG_experiment::robotService.robotServiceSetMoveRelativeParam(TENG_experiment::relativeMoveOnBase);
    }
    else if(cmd == 5) // qiao +45 degree
    {
        exit(1);
    }
    else if(cmd == 6) // qiao -45 degree
    {
        exit(1);
    }

    aubo_robot_namespace::wayPoint_S centerOfMindWayPoint;
    TENG_experiment::robotService.robotServiceGetCurrentWaypointInfo(centerOfMindWayPoint);
    int ret = TENG_experiment::robotService.robotServiceLineMove(centerOfMindWayPoint.jointpos, true);
    if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cerr<<"Move failed.　ret:"<<ret<<std::endl;
        exit(1);
    }
}


void TENG_experiment::horizontal_movement()
{
//    ServiceInterface robotService;

    int ret = aubo_robot_namespace::InterfaceCallSuccCode;

    /** Interface call: login ***/
    ret = TENG_experiment::robotService.robotServiceLogin(SERVER_HOST, SERVER_PORT, "aubo", "123456");
    if(ret == aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cout<<"login successful."<<std::endl;
    }
    else
    {
        std::cerr<<"login failed."<<std::endl;
    }


    /** If the real robot arm is connected, the arm needs to be initialized.**/
    aubo_robot_namespace::ROBOT_SERVICE_STATE result;

    //Tool dynamics parameter
    aubo_robot_namespace::ToolDynamicsParam toolDynamicsParam;
    memset(&toolDynamicsParam, 0, sizeof(toolDynamicsParam));

    ret = TENG_experiment::robotService.rootServiceRobotStartup(toolDynamicsParam/**Tool dynamics parameter**/,
                                               6        /*Collision level*/,
                                               true     /*Whether to allow reading poses defaults to true*/,
                                               true,    /*Leave the default to true */
                                               1000,    /*Leave the default to 1000 */
                                               result); /*Robot arm initialization*/
    if(ret == aubo_robot_namespace::InterfaceCallSuccCode)
    {
        std::cout<<"Robot arm initialization succeeded."<<std::endl;
    }
    else
    {
        std::cerr<<"Robot arm initialization failed."<<std::endl;
    }

    /** Business block **/
    /** Interface call: Initialize motion properties ***/
    TENG_experiment::robotService.robotServiceInitGlobalMoveProfile();

    /** Interface call: Set the maximum acceleration of the articulated motion ***/
    aubo_robot_namespace::JointVelcAccParam jointMaxAcc;
    jointMaxAcc.jointPara[0] = 50.0/180.0*M_PI;
    jointMaxAcc.jointPara[1] = 50.0/180.0*M_PI;
    jointMaxAcc.jointPara[2] = 50.0/180.0*M_PI;
    jointMaxAcc.jointPara[3] = 50.0/180.0*M_PI;
    jointMaxAcc.jointPara[4] = 50.0/180.0*M_PI;
    jointMaxAcc.jointPara[5] = 50.0/180.0*M_PI;   ////The interface requires the unit to be radians
    TENG_experiment::robotService.robotServiceSetGlobalMoveJointMaxAcc(jointMaxAcc);

    /** Interface call: set the maximum speed of articulated motion ***/
    aubo_robot_namespace::JointVelcAccParam jointMaxVelc;
    jointMaxVelc.jointPara[0] = 50.0/180.0*M_PI;
    jointMaxVelc.jointPara[1] = 50.0/180.0*M_PI;
    jointMaxVelc.jointPara[2] = 50.0/180.0*M_PI;
    jointMaxVelc.jointPara[3] = 50.0/180.0*M_PI;
    jointMaxVelc.jointPara[4] = 50.0/180.0*M_PI;
    jointMaxVelc.jointPara[5] = 50.0/180.0*M_PI;   ////The interface requires the unit to be radians
    TENG_experiment::robotService.robotServiceSetGlobalMoveJointMaxVelc(jointMaxVelc);


    /** Interface call: Initialize motion properties ***/
    TENG_experiment::robotService.robotServiceInitGlobalMoveProfile();

    /** Robot arm movement to zero posture **/
    double endMoveMaxAcc;
    endMoveMaxAcc = 0.2;   //Units m/s2
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineAcc(endMoveMaxAcc);
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxAngleAcc(endMoveMaxAcc);


    /** Interface call: Set the maximum speed of the end type motion Linear motion belongs to the end type motion***/
    double endMoveMaxVelc;
    endMoveMaxVelc = 0.2;   //Units m/s
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineVelc(endMoveMaxVelc);
    TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxAngleVelc(endMoveMaxVelc);

    double jointAngle[aubo_robot_namespace::ARM_DOF];

    for(int i=0;i<1;i++)
    {
//        //Preparation point joint movement is joint movement
//        robotService.robotServiceInitGlobalMoveProfile();

//        robotService.robotServiceSetGlobalMoveJointMaxAcc(jointMaxAcc);
//        robotService.robotServiceSetGlobalMoveJointMaxVelc(jointMaxVelc);
//        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        ret = robotService.robotServiceJointMove(jointAngle, true);   //Joint movement to preparation point
//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"MoveJoint failed.　ret:"<<ret<<std::endl;
//        }


//        //Arc
//        robotService.robotServiceInitGlobalMoveProfile();

//        robotService.robotServiceSetGlobalMoveEndMaxLineAcc(endMoveMaxAcc);
//        robotService.robotServiceSetGlobalMoveEndMaxAngleAcc(endMoveMaxAcc);
//        robotService.robotServiceSetGlobalMoveEndMaxLineVelc(endMoveMaxVelc);
//        robotService.robotServiceSetGlobalMoveEndMaxAngleVelc(endMoveMaxVelc);
//        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);

//        Util::initJointAngleArray(jointAngle,0.200000, -0.127267, -1.321122, 0.376934, -1.570794, -0.000008);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);

//        Util::initJointAngleArray(jointAngle,0.600000, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);

//        robotService.robotServiceSetGlobalCircularLoopTimes(0);    //Circle number
//        ret = robotService.robotServiceTrackMove(aubo_robot_namespace::ARC_CIR, true);

//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"TrackMove failed.　ret:"<<ret<<std::endl;
//        }


//        //Preparation point
//        robotService.robotServiceInitGlobalMoveProfile();

//        robotService.robotServiceSetGlobalMoveJointMaxAcc(jointMaxAcc);
//        robotService.robotServiceSetGlobalMoveJointMaxVelc(jointMaxVelc);
//        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        ret = robotService.robotServiceJointMove(jointAngle, true);   //Joint movement to preparation point
//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"Movevfailed.　ret:"<<ret<<std::endl;
//        }

//        //circle
//        robotService.robotServiceInitGlobalMoveProfile();

//        robotService.robotServiceSetGlobalMoveEndMaxLineAcc(endMoveMaxAcc);
//        robotService.robotServiceSetGlobalMoveEndMaxAngleAcc(endMoveMaxAcc);
//        robotService.robotServiceSetGlobalMoveEndMaxLineVelc(endMoveMaxVelc);
//        robotService.robotServiceSetGlobalMoveEndMaxAngleVelc(endMoveMaxVelc);
//        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);
//        Util::initJointAngleArray(jointAngle,-0.211675, -0.325189, -1.466753, 0.429232, -1.570794, -0.211680);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);
//        Util::initJointAngleArray(jointAngle,-0.037186, -0.224307, -1.398285, 0.396819, -1.570796, -0.037191);
//        robotService.robotServiceAddGlobalWayPoint(jointAngle);

//        robotService.robotServiceSetGlobalCircularLoopTimes(1);    //Circle number
//        ret = robotService.robotServiceTrackMove(aubo_robot_namespace::ARC_CIR,true);
//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"TrackMove failed.　ret:"<<ret<<std::endl;
//        }


        //Preparation point
//        robotService.robotServiceInitGlobalMoveProfile();

//        robotService.robotServiceSetGlobalMoveJointMaxAcc(jointMaxAcc);
//        robotService.robotServiceSetGlobalMoveJointMaxVelc(jointMaxVelc);
//        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
//        ret = robotService.robotServiceJointMove(jointAngle, true);   //Joint movement to preparation point
//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"MoveJoint failed.　ret:"<<ret<<std::endl;
//        }

        //MoveP
        TENG_experiment::robotService.robotServiceInitGlobalMoveProfile();

        TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineAcc(endMoveMaxAcc);
        TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxAngleAcc(endMoveMaxAcc);
        TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxLineVelc(endMoveMaxVelc);
        TENG_experiment::robotService.robotServiceSetGlobalMoveEndMaxAngleVelc(endMoveMaxVelc);
        Util::initJointAngleArray(jointAngle,-0.000003, -0.127267, -1.321122, 0.376934, -1.570796, -0.000008);
        TENG_experiment::robotService.robotServiceAddGlobalWayPoint(jointAngle);
        Util::initJointAngleArray(jointAngle,0.100000, -0.147267, -1.321122, 0.376934, -1.570794, -0.000008);
        TENG_experiment::robotService.robotServiceAddGlobalWayPoint(jointAngle);
        Util::initJointAngleArray(jointAngle,0.200000, -0.167267, -1.321122, 0.376934, -1.570796, -0.000008);
        TENG_experiment::robotService.robotServiceAddGlobalWayPoint(jointAngle);

//        robotService.robotServiceSetGlobalBlendRadius(0.03);                     //Blending radius
//        ret = robotService.robotServiceTrackMove(aubo_robot_namespace::CARTESIAN_GNUBSPLINEINTP,true);
//        if(ret != aubo_robot_namespace::InterfaceCallSuccCode)
//        {
//            std::cerr<<"TrackMove failed.　ret:"<<ret<<std::endl;
//        }
    }

    /** Robotic arm shutdown**/
    TENG_experiment::robotService.robotServiceRobotShutdown();

    /** Interface call: logout　**/
    TENG_experiment::robotService.robotServiceLogout();
}




