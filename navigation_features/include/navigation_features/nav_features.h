
#ifndef NAV_FEATURES_
#define NAV_FEATURES_

#include <math.h>

//ROS
#include <ros/ros.h>
#include <costmap_2d/costmap_2d.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <costmap_2d/cost_values.h>
#include <costmap_2d/costmap_2d_publisher.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/Pose2D.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <nav_msgs/OccupancyGrid.h>
#include <upo_msgs/PersonPoseUPO.h>
#include <upo_msgs/PersonPoseArrayUPO.h>
#include <geometry_msgs/PoseStamped.h>
#include <sensor_msgs/PointCloud.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <laser_geometry/laser_geometry.h>
#include <nav_msgs/GetMap.h>
//PCL
#include <pcl/common/transforms.h>
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include "pcl_ros/transforms.h"
#include <pcl/register_point_struct.h>

//Boost
#include <boost/thread.hpp>  // Mutex 

//OpenCV
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//UVA features
//#include <navigation_features/uva_features.h>



namespace features {

	class NavFeatures {

		public:
		
			enum gaussian_type{
				FRONT, 
				BACK, 
				LEFT, 
				RIGHT, 
				AROUND, 
				APPROACH,
			};
			
			enum dist_type{LINEAR_INC,LOG_INC,EXP_INC,INVERSE_DEC,LOG_DEC,EXP_DEC};

			NavFeatures();
			
			//Used in case of uva_features
			NavFeatures(tf::TransformListener* tf, float size_x, float size_y);

			~NavFeatures();
			
			NavFeatures(tf::TransformListener* tf, const costmap_2d::Costmap2D* loc_costmap, const costmap_2d::Costmap2D* glob_costmap, std::vector<geometry_msgs::Point>* footprint, float insc_radius, float size_x, float size_y);

			bool poseValid(geometry_msgs::PoseStamped* pose);

			float getCost(geometry_msgs::PoseStamped* s);
		
			std::vector<float> getFeatures(geometry_msgs::PoseStamped* s);

			void peopleCallback(const upo_msgs::PersonPoseArrayUPO::ConstPtr& msg);
			
			void pcCallback(const sensor_msgs::PointCloud2::ConstPtr& pc_in);
			
			void calculateGaussians();

			//Implemented for learning purposes
			void setPeople(upo_msgs::PersonPoseArrayUPO p);

			float costmapPointCost(float x, float y) const;
		
			//Feature: Distance to the goal
			float goalDistFeature(geometry_msgs::PoseStamped* s);
		
			//Feature: Distance to the closest obstacle (based on costmaps or map image)
			float obstacleDistFeature(geometry_msgs::PoseStamped* s);
		
			//Feature: Proxemics
			float proxemicsFeature(geometry_msgs::PoseStamped* s);
			float getProxemicsCost(float rx, float ry);		
			float gaussian_function(float x, float y, bool fr);
			float gaussian_function(float x, float y, float sx, float sy);

			geometry_msgs::PoseStamped transformPoseTo(geometry_msgs::PoseStamped pose_in, std::string frame_out, bool usetime);
		
			bool isQuaternionValid(const geometry_msgs::Quaternion q);
			
			float normalizeAngle(float val, float min, float max);
		
			void setWeights(std::vector<float> we) {
				w_.clear();
				w_ = we;
			}

			void setGoal(geometry_msgs::PoseStamped g); 
			
			
			//For laser projection
			void setupProjection();
			float distance_functions(const float distance, const dist_type type);
			void updateDistTransform();
			std::vector<int> worldToMap(geometry_msgs::Point32* world_point,nav_msgs::MapMetaData* map_metadata);


		private:
		
		
			struct gaussian {	
				float x;
				float y;
				float th;
				float sx;
				float sy;
				gaussian_type type;
			};
			
			
			struct group_candidate {
				geometry_msgs::Pose2D position;
				geometry_msgs::Pose2D interaction_point;
				int group;
			};
			
			ros::Publisher pub_gaussian_markers_;
			
			std::vector<gaussian> gaussians_;
			boost::mutex gaussianMutex_;

			//For laser projection
			bool use_laser_projection_;
			ros::Subscriber sub_pc_;
			cv::Mat map_image_;
			cv::Mat distance_transform_;
			nav_msgs::MapMetaData map_metadata_;
			double resolution_;
			std::vector<float> origin_;
			tf::TransformListener* listener_;
			laser_geometry::LaserProjection projector_;
			sensor_msgs::PointCloud2 laser_cloud_;
			boost::mutex laserMutex_;
			boost::mutex dtMutex_;
			int people_paint_area_; // the amount of pixels to be painted over at the presence of people
			float max_cost_obs_;
			
			
			bool use_uva_features_;
			//uva_cost_functions::UvaFeatures* uva_features_;
			
			
			const costmap_2d::Costmap2D* costmap_local_;
			const costmap_2d::Costmap2D* costmap_global_;
			tf::TransformListener* tf_listener_;
			std::vector<geometry_msgs::Point>* myfootprint_;
			float insc_radius_robot_;
			geometry_msgs::PoseStamped goal_;
			float max_planning_dist_;
			//float max_dist_2_;
			float size_x_;
			float size_y_;

			ros::NodeHandle nh_;
		
			// list of person objects	
			std::vector<upo_msgs::PersonPoseUPO> people_;
			ros::Subscriber sub_people_;
			boost::mutex peopleMutex_;
			std::string people_frame_id_;
			
			
			bool grouping_;
			float stddev_group_;
			float grouping_distance_;
			
		
			//parameters of the Gaussian functions
			float amp_;
			std::vector<float> sigmas_;
		
			//Weights to balance the costs
			std::vector<float> w_;

	};

}
#endif