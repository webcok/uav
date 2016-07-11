#include <ros/ros.h>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <thread>
#include <map>


class UAVException : public std::exception {
	private:
		std::string _message;
	public:
		UAVException(std::string m):_message(m) { }
		UAVException(char* m):_message(m) { }
		UAVException(const char* m):_message(m) { }
		virtual const char* what() const throw() {
			return (this->_message).c_str();
		}

		virtual ~UAVException() throw() {
			
		}
};


class Pose {
	private:
		double _pos_x;
		double _pos_y;
		double _pos_z;
		double _ori_x;
		double _ori_y;
		double _ori_z;

	public:
		Pose() {
		}
		Pose(double x, double y, double z, double ox, double oy, double oz):_pos_x(x), _pos_y(y), _pos_z(z), _ori_x(ox), _ori_y(oy), _ori_z(oz) { }
		Pose(const Pose& pose) {
			this->_pos_x = pose._pos_x;
			this->_pos_y = pose._pos_y;
			this->_pos_z = pose._pos_z;
			this->_ori_x = pose._ori_x;
			this->_ori_y = pose._ori_y;
			this->_ori_z = pose._ori_z;
		}
		Pose operator=(const Pose& pose) {
			if(this != &pose) {
				this->_pos_x = pose._pos_x;
				this->_pos_y = pose._pos_y;
				this->_pos_z = pose._pos_z;
				this->_ori_x = pose._ori_x;
				this->_ori_x = pose._ori_x;
				this->_ori_x = pose._ori_x;
			}
			return *this;
		}
		double get_pos_x() { return this->_pos_x; }
		double get_pos_y() { return this->_pos_y; }
		double get_pos_z() { return this->_pos_z; }
		double get_ori_x() { return this->_ori_x; }
		double get_ori_y() { return this->_ori_y; }
		double get_ori_z() { return this->_ori_z; }

		void set_pos_x(double val) { this->_pos_x = val; }
		void set_pos_y(double val) { this->_pos_y = val; }
		void set_pos_z(double val) { this->_pos_z = val; }
		void set_ori_x(double val) { this->_ori_x = val; }
		void set_ori_y(double val) { this->_ori_y = val; }
		void set_ori_z(double val) { this->_ori_z = val; }
};

class Wait {
	private:
		std::vector<int> _for;
		std::vector<int> _except;
	public:
		void add_for(int val) {
			if((this->_except).size() != 0) {
				throw UAVException("<for> and <except> are mutually exclusive.");
			}
			this->_for.push_back(val);
		}

		void add_except(int val) {
			if((this->_for).size() != 0) {
				throw UAVException("<for> and <except> are mutually exclusive.");
			}
			this->_except.push_back(val);
		}

		Wait() {
		}
};

class Action {
	private:
		int _id;
		Pose _goto_pose;
		Wait _wait;
	public:

		Action* next;

		Action(int i):_id(i) { 
			next = NULL;
		}

		void set_goto_pose(Pose pose) {
			this->_goto_pose = pose;
		}

		void add_wait_for(int val) {
			(this->_wait).add_for(val);
		}

		void add_wait_except(int val) {
			(this->_wait).add_except(val);
		}

};

class ActionList {
	private:
		Action* _head;
		Action* _tail;
	public:
		ActionList() {
			_head = NULL;
			_tail = NULL;
		}

		void add_action(Action& action) {
			if(this->_head == NULL) {
				action.next = NULL;
				this->_head = &action;
				this->_tail = &action;
			} else {
				this->_tail->next = &action;
				this->_tail = &action;
				this->_tail->next = NULL;
			}
		}

		Action* get_head() { //also removes it.
			Action* ret = this->_head;
			if(this->_head != NULL) {
				this->_head = this->_head->next;
				if(this->_head == NULL) {
					this->_tail = NULL;
				}
			}
			return ret;
		}

		bool has_action() {
			return this->_head != NULL;
		}


};

class UAV {
	private:
		ActionList _action_list;
		std::string _name;
		Pose _spawn_point;
	public:
		UAV(std::string n):_name(n) {
		}

		bool has_action_left() {
			return (this->_action_list).has_action();
		}

		void set_spawn_pose(double x, double y, double z, double ox, double oy, double oz) {
			_spawn_point.set_pos_x(x);
			_spawn_point.set_pos_y(y);
			_spawn_point.set_pos_z(z);
			_spawn_point.set_ori_x(ox);
			_spawn_point.set_ori_y(oy);
			_spawn_point.set_ori_z(oz);
		}

		void add_action(Action& action) {
			(this->_action_list).add_action(action);
		}
};


void drive_UAV(UAV uav) {
	while(uav.has_action_left()) {
	}
}


void populate(ros::NodeHandle& nh) {
	rapidxml::file<> file("/home/baskin/KOVAN/src/uav/uav_src/sample.xml");
	rapidxml::xml_document<> xml_doc;
	xml_doc.parse<rapidxml::parse_default>(file.data());
	rapidxml::xml_node<> *plan_node = xml_doc.first_node("plan");
	for(rapidxml::xml_node<> *uav_node = plan_node->first_node("uav"); uav_node; uav_node = uav_node->next_sibling()) {
		std::string id = "";
		for(rapidxml::xml_attribute<> *uav_attr = uav_node->first_attribute(); uav_attr; uav_attr = uav_attr->next_attribute()) {
			if(uav_attr->name() == "id") {
				id = uav_attr->value();
			}
		}

		if(id == "") {
			throw UAVException("No ID specified for the UAV");
		}

		std::string name = "/uav" + id;
		UAV uav(name);

		rapidxml::xml_node<> *spawn_node = uav_node->first_node("spawn");
		if(!spawn_node) {
			throw UAVException(std::string("No spawn point specified for UAV ") + name);
		}

		double s_x, s_y, s_z, s_ox, s_oy, s_oz;

		rapidxml::xml_node<> *pose_node = spawn_node->first_node("pos_x");
		if(!pose_node) {
			throw UAVException(std::string("No x spawn coordinate specified for UAV ") + name);
		}
		s_x = std::stod(pose_node->value());

		pose_node = spawn_node->first_node("pos_y");
		if(!pose_node) {
			throw UAVException(std::string("No y spawn coordinate specified for UAV ") + name);
		}
		s_y = std::stod(pose_node->value());

		pose_node = spawn_node->first_node("pos_z");
		if(!pose_node) {
			throw UAVException(std::string("No z spawn coordinate specified for UAV ") + name);
		}
		s_z = std::stod(pose_node->value());

		pose_node = spawn_node->first_node("ori_x");
		if(!pose_node) {
			throw UAVException(std::string("No x spawn orientation specified for UAV ") + name);
		}
		s_ox = std::stod(pose_node->value());

		pose_node = spawn_node->first_node("ori_y");
		if(!pose_node) {
			throw UAVException(std::string("No y spawn orientation specified for UAV ") + name);
		}
		s_oy = std::stod(pose_node->value());

		pose_node = spawn_node->first_node("ori_z");
		if(!pose_node) {
			throw UAVException(std::string("No z spawn orientation specified for UAV ") + name);
		}
		s_oz = std::stod(pose_node->value());

		uav.set_spawn_pose(s_x, s_y, s_z, s_ox, s_oy, s_oz);

		for(rapidxml::xml_node<> *action_node = uav_node->first_node("action"); action_node; action_node = action_node->next_sibling()) {
			
		}

	}
	
}

int main(int argc, char* argv[]) {
	ros::init(argc, argv, "uav_master");
	ros::NodeHandle nh;

	populate(nh);

	return 0;
}

