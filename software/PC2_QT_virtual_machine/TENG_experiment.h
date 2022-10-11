// author: Zenan-SSR
// date: 2022/10/11
// descrition: Using esp32 uart control AUBO movement

#ifndef TENG_EXPERIMENT_H
#define TENG_EXPERIMENT_H

#include "AuboRobotMetaType.h"
#include "serviceinterface.h"

class TENG_experiment
{
public:
    /**
     * @brief demo
     *
     * Trajectory movement
     *
     */
    ServiceInterface robotService;
    double jointAngle[aubo_robot_namespace::ARM_DOF];
    aubo_robot_namespace::JointStatus jointStatus[6];
    aubo_robot_namespace::MoveRelative relativeMoveOnBase;

    TENG_experiment(); // Constructor function
    ~TENG_experiment(); // Deconstructor function

    void vertical_movement();

    void target_movement(int cmd, double step);

    void horizontal_movement();
};

#endif // TENG_EXPERIMENT_H
