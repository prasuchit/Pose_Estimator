#include "stdafx.h"
#include "tricycle_params.h"

tricycle_params::tricycle_params() {
}

void head_ang_dot(const state_type &h_ang_init, state_type &dxdt, int t)
{
	dxdt[0] = h_a_d;
}

void disp_ang(const state_type &x, const double t)
{
	head = x[0];
}

void xdot(const state_type &h_ang_init, state_type &dxdt, int t)
{
	dxdt[0] = x_d;
}

void disp_x(const state_type &x, const double t)
{
	xpose = x[0];
}

void ydot(const state_type &h_ang_init, state_type &dxdt, int t)
{
	dxdt[0] = y_d;
}

void disp_y(const state_type &x, const double t)
{
	ypose = x[0];
}

tuple<float, float, float> tricycle_params::estimate(int time, float steering_angle, int encoder_ticks, int ang_vel) {
	float vel_x = ((2*M_PI*front_wheel_radius)*(encoder_ticks/encoder_resolution)) / (time);	//velocity of front wheel in the current orientation
	float vel_linear = vel_x*cos(steering_angle);	//linear velocity of front wheel along the body
	heading_angle = ((vel_x / wheelbase)*sin(steering_angle));
	h_a_d = heading_angle;
	integrate_const(stepper_type(), head_ang_dot, h_ang_init, t_init, (double)time, 0.1, disp_ang);	//Integrating angular vel to get heading angle
	x_platform = (vel_linear*cos(head));	//x coordinate of platform
	y_platform = (vel_linear*sin(head));	//y coordinate of platform
	return std::make_tuple(x_platform, y_platform, heading_angle);	//return the tuple
}

int main(int argc, char ** argv) {
	tricycle_params tp;
	while (repeat == 'y'){
		cout << "Enter the time taken for the measurement in seconds: ";
		cin >> t_in_sec;
		if (t_in_sec<=0) {
			cout << "Invalid time value";
			break;
		}
		cout << "\nEnter the steering angle in radians (range:-1.5708 to 1.5708) ";
		cin >> st_ang;
		if (st_ang<-1.5708 || st_ang>1.5708) {
			cout<< "Steering angle beyond acceptable range";
			break;
		}
		cout << "\nEnter the encoder ticks as an integer value (512 ticks correspond to one revolution): ";
		cin >> enc_ticks;
		cout << "\nEnter the angular velocity in radians/sec: ";
		cin >> ang_vel;
		tp.est_pose = tp.estimate(t_in_sec, st_ang, enc_ticks, ang_vel);	//function call
		//Integrating x dot and y dot from time t_init to t_in sec
		x_d = tp.x_platform;
		y_d = tp.y_platform;
		integrate_const(stepper_type(), xdot, tp.x_init, tp.t_init, t_in_sec, 0.1, disp_x);
		integrate_const(stepper_type(), ydot, tp.y_init, tp.t_init, t_in_sec, 0.1, disp_y);
		if (head > 6.283) { head /= 6.283; head = modf(head, &temp)*6.283; }	//converting angles to be within 360 degrees 
		//Updating time, x and y coordinate for the next iteration
		tp.t_init = t_in_sec;
		tp.x_init[0] = xpose;	
		tp.y_init[0] = ypose;
		tp.estimated_pose = std::make_tuple(xpose, ypose, head);	//Pose update tuple
		cout << "\nX coordinate, Y coordinate and Heading angle are: (" << xpose << ", " << ypose << ", " << head << ")";
		cout << "\nDo you want to perform another iteration? (y/n) ";
		cin >> repeat;
	}
	_getch();
}

tricycle_params::~tricycle_params() {
}

