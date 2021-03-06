/*********************************************************************
*
* Author: Fernando Caballero Benítez
*********************************************************************/
#ifndef __TRAJECTORY_PLANNER_PP_H__
#define __TRAJECTORY_PLANNER_PP_H__

#include <vector>
#include <cmath>

//for obstacle data access
#include <costmap_2d/costmap_2d.h>
#include <costmap_2d/cost_values.h>
#include <upo_navigation/footprint_helper.h>

#include <upo_navigation/world_model.h>
#include <upo_navigation/trajectory.h>
#include <upo_navigation/Position2DInt.h>
//#include <teresa_navigation/BaseLocalPlannerConfig.h>

//we'll take in a path as a vector of poses
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/Twist.h>

//for some datatypes
#include <tf/transform_datatypes.h>

//for creating a local cost grid
#include <upo_navigation/map_cell.h>
#include <upo_navigation/map_grid.h>

namespace upo_nav {
  /**
   * @class TrajectoryPlanner
   * @brief Computes control velocities for a robot given a costmap, a plan, and the robot's position in the world. 
   */
  class TrajectoryPlannerPP{
    //friend class TrajectoryPlannerTest; //Need this for gtest to work
    public:

	  TrajectoryPlannerPP(WorldModel& world_model, 
          const costmap_2d::Costmap2D& costmap, 
          std::vector<geometry_msgs::Point> footprint_spec,
		  double controller_freq);


      /**
       * @brief  Constructs a trajectory controller
       * @param world_model The WorldModel the trajectory controller uses to check for collisions 
       * @param costmap A reference to the Costmap the controller should use
       * @param footprint_spec A polygon representing the footprint of the robot. (Must be convex)
       * @param inscribed_radius The radius of the inscribed circle of the robot
       * @param circumscribed_radius The radius of the circumscribed circle of the robot
       * @param acc_lim_x The acceleration limit of the robot in the x direction
       * @param acc_lim_y The acceleration limit of the robot in the y direction
       * @param acc_lim_theta The acceleration limit of the robot in the theta direction
       * @param sim_time The number of seconds to "roll-out" each trajectory
       * @param sim_granularity The distance between simulation points should be small enough that the robot doesn't hit things
       * @param vx_samples The number of trajectories to sample in the x dimension
       * @param vtheta_samples The number of trajectories to sample in the theta dimension
       * @param pdist_scale A scaling factor for how close the robot should stay to the path
       * @param gdist_scale A scaling factor for how aggresively the robot should pursue a local goal
       * @param occdist_scale A scaling factor for how much the robot should prefer to stay away from obstacles
       * @param heading_lookahead How far the robot should look ahead of itself when differentiating between different rotational velocities
       * @param oscillation_reset_dist The distance the robot must travel before it can explore rotational velocities that were unsuccessful in the past
       * @param escape_reset_dist The distance the robot must travel before it can exit escape mode
       * @param escape_reset_theta The distance the robot must rotate before it can exit escape mode
       * @param holonomic_robot Set this to true if the robot being controlled can take y velocities and false otherwise
       * @param max_vel_x The maximum x velocity the controller will explore
       * @param min_vel_x The minimum x velocity the controller will explore
       * @param max_vel_th The maximum rotational velocity the controller will explore
       * @param min_vel_th The minimum rotational velocity the controller will explore
       * @param min_in_place_vel_th The absolute value of the minimum in-place rotational velocity the controller will explore
       * @param backup_vel The velocity to use while backing up
       * @param dwa Set this to true to use the Dynamic Window Approach, false to use acceleration limits
       * @param heading_scoring Set this to true to score trajectories based on the robot's heading after 1 timestep
       * @param heading_scoring_timestep How far to look ahead in time when we score heading based trajectories
       * @param meter_scoring adapt parameters to costmap resolution
       * @param simple_attractor Set this to true to allow simple attraction to a goal point instead of intelligent cost propagation
       * @param y_vels A vector of the y velocities the controller will explore
       * @param angular_sim_granularity The distance between simulation points for angular velocity should be small enough that the robot doesn't hit things
       */
      /*TrajectoryPlannerPP(WorldModel& world_model, 
          const costmap_2d::Costmap2D& costmap, 
          std::vector<geometry_msgs::Point> footprint_spec,
          double acc_lim_x = 1.0, double acc_lim_y = 1.0, double acc_lim_theta = 1.0,
          double sim_time = 1.0, double sim_granularity = 0.025, 
          double pdist_scale = 0.6, double gdist_scale = 0.8, double occdist_scale = 0.2,
          double max_vel_x = 0.5, double min_vel_x = 0.1, 
          double max_vel_th = 1.0, double min_vel_th = 1.0, double min_in_place_vel_th = 0.4,
          bool heading_scoring = false, double heading_scoring_timestep = 0.1,
          bool simple_attractor = false,
          double angular_sim_granularity = 0.025,
          double goal_lin_tolerance = 0.3, double goal_ang_tolerance = 0.2, double wp_tolerance = 0.2,
		  double controller_freq = 15.0);*/

      /**
       * @brief  Destructs a trajectory controller
       */
      ~TrajectoryPlannerPP();

      /**
       * @brief Reconfigures the trajectory planner
       */
      //void reconfigure(BaseLocalPlannerConfig &cfg);

      /**
       * @brief  Given the current position, orientation, and velocity of the robot, return a trajectory to follow
       * @param global_pose The current pose of the robot in world space 
       * @param global_vel The current velocity of the robot in world space
       * @param drive_velocities Will be set to velocities to send to the robot base
       * @return True if a valid command was found, false otherwise
       */
      bool findBestAction(tf::Stamped<tf::Pose> global_pose, tf::Stamped<tf::Pose> global_vel,
          geometry_msgs::Twist& cmd_vel);

      /**
       * @brief  Update the plan that the controller is following
       * @param new_plan A new plan for the controller to follow 
       * @param compute_dists Wheter or not to compute path/goal distances when a plan is updated
       */
      bool updatePlan(const std::vector<geometry_msgs::PoseStamped>& new_plan, bool compute_dists = false);

	  bool isGoalReached();
	  void resetGoal();

      /**
       * @brief  Accessor for the goal the robot is currently pursuing in world corrdinates
       * @param x Will be set to the x position of the local goal 
       * @param y Will be set to the y position of the local goal 
       */
      void getLocalGoal(double& x, double& y);

      /**
       * @brief  Generate and score a single trajectory
       * @param x The x position of the robot  
       * @param y The y position of the robot  
       * @param theta The orientation of the robot
       * @param vx The x velocity of the robot
       * @param vy The y velocity of the robot
       * @param vtheta The theta velocity of the robot
       * @param vx_samp The x velocity used to seed the trajectory
       * @param vy_samp The y velocity used to seed the trajectory
       * @param vtheta_samp The theta velocity used to seed the trajectory
       * @return True if the trajectory is legal, false otherwise
       */
      bool checkTrajectory(double x, double y, double theta, double vx, double vy, 
          double vtheta, double vx_samp, double vy_samp, double vtheta_samp);

      /**
       * @brief  Generate and score a single trajectory
       * @param x The x position of the robot  
       * @param y The y position of the robot  
       * @param theta The orientation of the robot
       * @param vx The x velocity of the robot
       * @param vy The y velocity of the robot
       * @param vtheta The theta velocity of the robot
       * @param vx_samp The x velocity used to seed the trajectory
       * @param vy_samp The y velocity used to seed the trajectory
       * @param vtheta_samp The theta velocity used to seed the trajectory
       * @return The score (as double)
       */
      double scoreTrajectory(double x, double y, double theta, double vx, double vy, 
          double vtheta, double vx_samp, double vy_samp, double vtheta_samp);

      /**
       * @brief Compute the components and total cost for a map grid cell
       * @param cx The x coordinate of the cell in the map grid
       * @param cy The y coordinate of the cell in the map grid
       * @param path_cost Will be set to the path distance component of the cost function
       * @param goal_cost Will be set to the goal distance component of the cost function
       * @param occ_cost Will be set to the costmap value of the cell
       * @param total_cost Will be set to the value of the overall cost function, taking into account the scaling parameters
       * @return True if the cell is traversible and therefore a legal location for the robot to move to
       */
      bool getCellCosts(int cx, int cy, float &path_cost, float &goal_cost, float &occ_cost, float &total_cost);

      /** @brief Set the footprint specification of the robot. */
      void setFootprint( std::vector<geometry_msgs::Point> footprint ) { footprint_spec_ = footprint; }

      /** @brief Return the footprint specification of the robot. */
      geometry_msgs::Polygon getFootprintPolygon() const { return costmap_2d::toPolygon(footprint_spec_); }
      std::vector<geometry_msgs::Point> getFootprint() const { return footprint_spec_; }

    private:

      /**
       * @brief  Generate and score a single trajectory
       * @param x The x position of the robot  
       * @param y The y position of the robot  
       * @param theta The orientation of the robot
       * @param vx The x velocity of the robot
       * @param vy The y velocity of the robot
       * @param vtheta The theta velocity of the robot
       * @param vx_samp The x velocity used to seed the trajectory
       * @param vy_samp The y velocity used to seed the trajectory
       * @param vtheta_samp The theta velocity used to seed the trajectory
       * @param acc_x The x acceleration limit of the robot
       * @param acc_y The y acceleration limit of the robot
       * @param acc_theta The theta acceleration limit of the robot
       * @param impossible_cost The cost value of a cell in the local map grid that is considered impassable
       * @param traj Will be set to the generated trajectory with its associated score 
       */
      void generateTrajectory(double x, double y, double theta, double vx, double vy, 
          double vtheta, double vx_samp, double vy_samp, double vtheta_samp, double acc_x, double acc_y,
          double acc_theta, double impossible_cost, Trajectory& traj);

      /**
       * @brief  Checks the legality of the robot footprint at a position and orientation using the world model
       * @param x_i The x position of the robot 
       * @param y_i The y position of the robot 
       * @param theta_i The orientation of the robot
       * @return 
       */
      double footprintCost(double x_i, double y_i, double theta_i);

      upo_nav::FootprintHelper footprint_helper_;
    
      MapGrid path_map_; ///< @brief The local map grid where we propagate path distance
      MapGrid goal_map_; ///< @brief The local map grid where we propagate goal distance
      const costmap_2d::Costmap2D& costmap_; ///< @brief Provides access to cost map information
	   
      WorldModel& world_model_; ///< @brief The world model that the controller uses for collision detection

      std::vector<geometry_msgs::Point> footprint_spec_; ///< @brief The footprint specification of the robot

      std::vector<geometry_msgs::PoseStamped> global_plan_; ///< @brief The global path for the robot to follow


      double sim_time_; ///< @brief The number of seconds each trajectory is "rolled-out"
      double sim_granularity_; ///< @brief The distance between simulation points
      double angular_sim_granularity_; ///< @brief The distance between angular simulation points

      double pdist_scale_, gdist_scale_, occdist_scale_; ///< @brief Scaling factors for the controller's cost function
      double acc_lim_x_, acc_lim_y_, acc_lim_th_; ///< @brief The acceleration limits of the robot 
      
      double max_vel_x_, min_vel_x_, max_vel_th_, min_vel_th_, min_in_place_vel_th_; ///< @brief Velocity limits for the controller

      bool heading_scoring_; ///< @brief Should we score based on the rollout approach or the heading approach
      double heading_scoring_timestep_; ///< @brief How far to look ahead in time when we score a heading
      bool simple_attractor_;  ///< @brief Enables simple attraction to a goal point

      double inscribed_radius_, circumscribed_radius_;


      //boost::mutex configuration_mutex_;
      
      // Pure pursuit params
      int wp_index_;
	  bool running_;
	  double start_x_;
	  double start_y_;
	  double start_t_;
	  double goal_x_;
	  double goal_y_;
	  double goal_t_;
	  double goal_lin_tolerance_;
	  double goal_ang_tolerance_;
	  double wp_tolerance_;
	  bool new_plan_;
	  double controller_freq_;
	  bool goal_reached_;

      /**
       * @brief  Compute x position based on velocity
       * @param  xi The current x position
       * @param  vx The current x velocity
       * @param  vy The current y velocity
       * @param  theta The current orientation
       * @param  dt The timestep to take
       * @return The new x position 
       */
      inline double computeNewXPosition(double xi, double vx, double vy, double theta, double dt){
        return xi + (vx * cos(theta) + vy * cos(M_PI_2 + theta)) * dt;
      }

      /**
       * @brief  Compute y position based on velocity
       * @param  yi The current y position
       * @param  vx The current x velocity
       * @param  vy The current y velocity
       * @param  theta The current orientation
       * @param  dt The timestep to take
       * @return The new y position 
       */
      inline double computeNewYPosition(double yi, double vx, double vy, double theta, double dt){
        return yi + (vx * sin(theta) + vy * sin(M_PI_2 + theta)) * dt;
      }

      /**
       * @brief  Compute orientation based on velocity
       * @param  thetai The current orientation
       * @param  vth The current theta velocity
       * @param  dt The timestep to take
       * @return The new orientation
       */
      inline double computeNewThetaPosition(double thetai, double vth, double dt){
        return thetai + vth * dt;
      }

      //compute velocity based on acceleration
      /**
       * @brief  Compute velocity based on acceleration
       * @param vg The desired velocity, what we're accelerating up to 
       * @param vi The current velocity
       * @param a_max An acceleration limit
       * @param  dt The timestep to take
       * @return The new velocity
       */
      inline double computeNewVelocity(double vg, double vi, double a_max, double dt){
        if((vg - vi) >= 0) {
          return std::min(vg, vi + a_max * dt);
        }
        return std::max(vg, vi - a_max * dt);
      }

      void getMaxSpeedToStopInTime(double time, double& vx, double& vy, double& vth){
        vx = acc_lim_x_ * std::max(time, 0.0);
        vy = acc_lim_y_ * std::max(time, 0.0);
        vth = acc_lim_th_ * std::max(time, 0.0);
      }

      double lineCost(int x0, int x1, int y0, int y1);
      double pointCost(int x, int y);
      double headingDiff(int cell_x, int cell_y, double x, double y, double heading);
  };
};

#endif
