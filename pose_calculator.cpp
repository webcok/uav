#include <ros/ros.h>
#include <uav/UavPose.h>
#include <geometry_msgs/Twist.h>
#include <stdlib.h>
#include <vector>






using namespace std;






bool isEqual(double a,double b){
	return abs(a-b)>0.01 ? false:true;
}





class PoseType{
	public:
		double x;
		double y;
		double z;
		PoseType operator=(const Pose& pose) {
			if(this != &pose){
				this->x = pose.x;
				this->y = pose.y;
				this->z = pose.z;
			}
			return *this;
		}
};
class PoseHelp{
	private:
		bool desired_updated;
		bool desired_achived;
		bool desired_set;
		bool current_set;
		PoseType current_linear;
		PoseType current_angular;
		PoseType desired_linear;
		PoseType desired_angular;
	public:
		PoseHelp(double x,double y,double z){
			desired_updated = false;
			desire_achived = false;
			desired_set = false;
			current_set = false;
			current_angular.x = 0;
			current_angular.y = 0;
			current_angular.z = 0;
			desired_angular.x = 0;
			desired_angular.y = 0;
			desired_angular.z = 0;
		}
		void CurrentUpdate(double x,double y,double z){
			current_linear.x = x;
			current_linear.y = y;
			current_linear.z = z;
			current_set = true;
		}
		void DesiredUpdate(double x,double y, double z){
			desired_linear.x = x;
			desired_linear.y = y;
			desired_linear.z = z;
			desired_set = true;
			desired_updated = true;
			desired_achived = false;
		}
		bool getDesiredSet(){
			return desired_set;
		}
		bool getCurrentSet(){
			return current_set;
		}
		bool shouldWait(){
			return (!desired_updated) && (desired_achived);
		}
		PoseType getDesiredLinear(){
			return desired_linear;
		}
		PoseType getCurrentLinear(){
			return current_linear;
		}
		float getxDifference(){
			if(isEqual(desired_linear.x, current_linear.x))
				return 0;
			else
				return (desired_linear.x - current_linear.x);
		}
		float getyDifference(){
			if(isEqual(desired_linear.y, current_linear.y))
				return 0;
			else
				return (desired_linear.y - current_linear.y);
	
		}
		float getzDifference(){
		if(isEqual(desired_linear.z, current_linear.z))
				return 0;
			else
				return (desired_linear.z - current_linear.z);
		}
		void setDesiredAchived(){
			desired_achived = true;
		}
};

PoseHelp pose_handle();
ros::Publisher pub;


//Updates the current Pose
void currentPose(const geometry_msgs::Twist &msg){
	pose_handle.CurrentUpdate(msg.linear.x, msg.linear.y, msg.linear.z);
	uav::UAVPose msg;
	if((pose_handle.getDesiredSet() == false) || (pose_handle.getCurrentSet() == false))
		return;
	if(pose_handle.shouldWait() == false){
		double x = poseHandle.getxDifference();
		double y = poseHandle.getyDifference();
		double z = poseHandle.getzDifference();
		// if UAV is in the correct position
		if( (x== 0) && (y==0) && (z==0)){
			pose_handle.setDesiredAchived();
		}
		else{
			msg.linear.x = x;
			msg.linear.y = y;
			msg.linear.z = z;
			pub.publish(msg);
		}
		 
	}
	
		
}

//Updates the desired Pose
void desiredPose(const uav::UAVPose& msg){
	pose_handle.DesiredUpdate(msg.linear.x, msg.linear.y, msg.linear.z);
}


int main(int argc,char** argv){
	ros::init(argc,argv,"pose_calculator");
	ros::NodeHandle nh;
	pub = nh.advertise<uav::UAVPose>("Difference",1000);
	ros::Subscriber sub_current = nh.subscribe("cmd_vel",1000,&currentPose); 
	ros::Subscriber sub_desired = nh.subscribe("DesiredPose",1000,&desiredPose);
	while(ros::ok()){
		ros.spinOnce();
		
	}
	return 0;
}