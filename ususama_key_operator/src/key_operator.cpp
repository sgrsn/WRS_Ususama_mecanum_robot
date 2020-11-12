/*******************************************************************************
Copyright (c) 2020, Hidaka Sato
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 
1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*******************************************************************************/
#include <fcntl.h>
#include <termios.h>
#include <ros/ros.h>
#include <geometry_msgs/Twist.h>

// ASCII code table
#define ESC   0x1b
#define SPACE 0x20
#define W     0x77
#define X     0x78
#define A     0x61
#define D     0x64
#define S     0x73
#define Z     0x7A
#define C     0x63

int getch(void)
{
  struct termios oldt, newt;
  int ch;

  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 1;
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

  return ch;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "key_operator");
  ros::NodeHandle node_handle("");
  float vel_step = 0.01;
  ros::Publisher cmd_vel_pub = node_handle.advertise<geometry_msgs::Twist>("wheel/cmd_vel", 10);
  geometry_msgs::Twist cmd_vel;
  std::string msg =
  "Moving around:\n"
  "        w\n"
  "   a    s    d\n"
  "   w         c\n"
  "\n"
  "w/s : +/- forward velocity\n"
  "a/d : +/- right velocity\n"
  "z/c : +/- angular velocity\n"
  "space : stop the robot\n"
  "\n"
  "CTRL-C : quit\n";
  ROS_INFO("%s", msg.c_str());

  ros::Rate loop_rate(50);
  while(ros::ok())
  {
    if (kbhit())
    {
      char c = getch();
      if (c == W)
      {
        cmd_vel.linear.y += vel_step;
      }
      else if (c == S)
      {
        cmd_vel.linear.y -= vel_step;
      }
      else if (c == A)
      {
        cmd_vel.linear.x -= vel_step;
      }
      else if (c == D)
      {
        cmd_vel.linear.x += vel_step;
      }
      else if (c == Z)
      {
        cmd_vel.angular.z += vel_step;
      }
      else if (c == C)
      {
        cmd_vel.angular.z -= vel_step;
      }
      else if (c == SPACE)
      {
        cmd_vel.linear.x  = 0;
        cmd_vel.linear.y  = 0;
        cmd_vel.angular.z = 0;
      }
      else
      {
      }
      cmd_vel_pub.publish(cmd_vel);
    }
    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}