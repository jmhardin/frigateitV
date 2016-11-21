#include <random>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <string.h>

#define SHOT_CONST 0.3

double health_factor(double cur_health)
{//takes current health fraction and returns strength modifier
	return .5 + .5*cur_health;
}
void shoot_frigates(
	std::vector<double> &frigate_healths,\
	double frigate_strength,\
	double shot_strength,\
	std::mt19937_64 &rgen)
{
	//Apply damage to first nonzero health frigate in list.  Do noting if all frigates are dead.
	std::uniform_real_distribution<double> distribution(.8,1.2);
	for (unsigned int i = 0; i < frigate_healths.size(); i++)
	{
		if (frigate_healths[i] < 0)
		{//Shoot next frigate
			continue;
		}
		else
		{
			frigate_healths[i] -= SHOT_CONST*distribution(rgen)*shot_strength/frigate_strength;
			break;
		}
	}
}
int run_frigate_sim(\
	std::vector<double> &frigate_healths, \
	int n_frigates, \
	double frigate_strength, \
	double city_strength, \
	std::vector<double> defender_strength, \
	double city_health, \
	std::mt19937_64 &rgen)
{
	//run rounds until city is dead or all frigates are dead
	//return number of rounds and modify frigate_healths to contain ending health
	std::uniform_real_distribution<double> distribution(.8,1.2);
	double tot_city_health = city_health;
	double cur_city_health = tot_city_health;
	int frigate_shots = 0;
	int turns = 0;
	for (int i = 0; i < n_frigates; i++)
	{
		frigate_healths.push_back(1.0);
	}
	
	while (true)
	{
		turns++;
		//All frigates shoot city
		frigate_shots = 0;
		for (unsigned int i = 0; i < frigate_healths.size(); i++)
		{	
			if (frigate_healths[i] > 0)
			{
				frigate_shots++;
				cur_city_health -= SHOT_CONST*distribution(rgen)*health_factor(frigate_healths[i])*frigate_strength/city_strength;
				//printf("%12.04f\n",distribution(rgen));
			}
		}
		if (cur_city_health < 0)
		{
			return turns;
		}
		if (frigate_shots == 0)
		{
			//ended last turn
			return turns-1;
		}

		//city heals: I think this is right
		cur_city_health += .2 + .05*(tot_city_health-2.0);

		//city shoots frigates:
		shoot_frigates(
			frigate_healths,\
			frigate_strength,\
			health_factor(cur_city_health/tot_city_health)*city_strength,\
			rgen);
	
		//defenders shoot frigates
		for (unsigned int i = 0; i < defender_strength.size(); i++)
		{
			shoot_frigates(
				frigate_healths,\
				frigate_strength,\
				defender_strength[i],\
				rgen);
		}
	}
		


}


int main( int argc, char** argv)
{
	std::random_device dev;
	std::mt19937_64 rgen(dev());
	int trials = 10;
	int n_frigates = 4;
	double city_strength = 35;
	double frigate_strength = 28;
	double city_health = 2.5;
	
	std::vector<double> defender_strength;
	
	bool sucessful_options = false;
	bool quiet_mode = false;
	bool trial_data = false;
	for (int i = 1; i < argc; i++)
	{
		sucessful_options = true;
		if (strcmp(argv[i],"-n") == 0)
		{
			i++;
			trials = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-d") == 0)
		{
			i++;
			defender_strength.push_back(atof(argv[i]));
		}
		else if (strcmp(argv[i],"-quiet_mode") == 0)
		{
			quiet_mode = true;
		}
		else if (strcmp(argv[i],"-trial_data") == 0)
		{
			trial_data = true;
		}
		else if (strcmp(argv[i],"-city_strength") == 0)
		{
			i++;
			city_strength = atof(argv[i]);
		}
		else if (strcmp(argv[i],"-frigate_strength") == 0)
		{
			i++;
			frigate_strength = atof(argv[i]);
		}
		else if (strcmp(argv[i],"-n_frigates") == 0)
		{
			i++;
			n_frigates = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"-city_health") == 0)
		{
			i++;
			city_health = atof(argv[i]);
		}
		else
		{
			sucessful_options = false;
			break;
		}
	}
	if (sucessful_options == false)
	{
		printf("USAGE:\n");
		printf("-n <trials>\n");
		printf("-city_strength <city_strength>\n");
		printf("-city_health <city_health>\n");
		printf("-frigate_strength <frigate_strength>\n");
		printf("-n_frigates <number of frigates>\n");
		printf("-d <defender_strength>\n\n");
		printf("-d may be appended repeatedly to simulate more defenders\n\n");
		printf("Flags:\n");
		printf("-quiet_mode - only output results (AVG_FRIGATES_LEFT AVG_TURNS AVG_SUCCESS)\n");
		printf("-trial_data - output results per trial (FRIGATES_LEFT TURNS) - does not output final averages\n");
		printf("\n\n\nNOTE: This sim assumes that your frigates do nothing but attack the city and success is on a 0 health city\n");
		printf("It also applys shots from defenders in the order listed - it is possible that more efficient shooting from your opponent could make you do worse (though probably only slightly).\n");
		return -1;
	}

	double avg_turns_taken = 0;
	double avg_success = 0;
	double avg_frigates_left = 0;

	if (quiet_mode == false)
	{
		printf("Begining %d trials\n",trials);
		printf("Frigates:           %d\n",n_frigates);
		printf("Frigate Strength:   %8.1f\n",frigate_strength);
		printf("City Strength:      %8.1f\n",city_strength);
		printf("City Health:        %8.1f\n",city_health);
		
		if (defender_strength.size() > 0)
		{
			printf("\nDefenders: \n");
			for (unsigned int i = 0; i < defender_strength.size(); i++)
			{
				printf("%8.1f ",defender_strength[i]);
			}
			printf("\n");
		}
		else
		{
			printf("\nNo Defenders\n");
		}
	}

	for (int i = 0; i < trials; i++)
	{
		bool city_taken = false;
		std::vector<double> frigate_healths;
		int turns = run_frigate_sim(\
			frigate_healths, \
			n_frigates, \
			frigate_strength, \
			city_strength, \
			defender_strength, \
			city_health, \
			rgen);

		avg_turns_taken += turns;

		double frigates_left = 0;
		for (unsigned int j = 0; j < frigate_healths.size(); j++)
		{
			if (frigate_healths[j] > 0)
			{
				frigates_left += frigate_healths[j];
				city_taken = true;
			}
		}

		avg_frigates_left += frigates_left;

		if (city_taken == true)
		{
			avg_success += 1.0;
		}

		if (trial_data == true)
		{
			printf("%2d %12.2f %s\n",turns,frigates_left,city_taken?"true":"false");
		}
	}
	avg_turns_taken /= trials;
	avg_frigates_left /= trials;
	avg_success /= trials;

	printf("       Turns     Frigates       Success\n");
	printf("%12.01f %12.01f %12.01f%%\n",avg_turns_taken,avg_frigates_left,avg_success*100);

	return 0;
}
