#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    int point_count; 
    cin >> point_count; cin.ignore();
    vector<vector<int>> surf_points(point_count, vector<int>(2));
    for (int i = 0; i < point_count; i++) {
        int point_x;
        int point_y; 
        cin >> point_x >> point_y; cin.ignore();
        surf_points[i][0] = point_x;
        surf_points[i][1] = point_y;
    }
    
    int land_start, land_end, land_height;
    for (size_t i = 0; i < point_count; i++) {
        if(surf_points[i][1] == surf_points[i+1][1] && surf_points[i+1][0] - surf_points[i][0] >= 1000) {
            land_start = surf_points[i][0];
            land_end = surf_points[i + 1][0];
            land_height = surf_points[i][1];
            break;
        }
    }

    while (1) {
        int position_x;
        int position_y;
        int hor_velocity;
        int vert_velocity;
        int remaining_fuel; 
        int cur_angle; 
        int cur_thrust; 
        cin >> position_x >> position_y >> hor_velocity >> vert_velocity >> remaining_fuel >> cur_angle >> cur_thrust; cin.ignore();

        int target_angle = cur_angle;
        int target_thrust = cur_thrust;

        if(position_x - land_start < -300 && hor_velocity < 40) {
            target_angle = -50;
            target_thrust = 4;
        }
        else if (position_x - land_start < -300 && hor_velocity >= 40 && hor_velocity <= 60) {
            target_angle = 0;
            if (vert_velocity < 2) target_thrust = 4;
            else target_thrust = 0;
        }
        else if (position_x - land_start < -300 && hor_velocity >= 60) {
            target_angle = 50;
            target_thrust = 4;
        }
        else if(position_x >= land_start && position_x <= land_end && hor_velocity > 5) {
            target_angle = 50;
            target_thrust = 4;
        }
        else if(position_x >= land_start && position_x <= land_end && hor_velocity < -5) {
            target_angle = -50;
            target_thrust = 4;
        }
        else if(position_x >= land_start && position_x <= land_end && hor_velocity > -5 && hor_velocity < 5) {
            target_angle = 0;
            target_thrust = 3;
            if (vert_velocity <= -40) target_thrust = 4;
            else target_thrust = 3;
        }
        else if(position_x - land_end > 300 && hor_velocity > -40) {
            target_angle = 50;
            target_thrust = 4;
        }
        else if (position_x - land_end > 300 && hor_velocity <= -40 && hor_velocity >= -60) {
            target_angle = 0;
            if (vert_velocity < 2) target_thrust = 4;
            else target_thrust = 0;
        }
        else if (position_x - land_end > 300 && hor_velocity <= -60) {
            target_angle = -50;
            target_thrust = 4;
        }
   
        cout << target_angle << " " << target_thrust << " " << endl;
    }
}