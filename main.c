#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef struct s_data
{
	int my_id;
	int total_count;
	int height;
	int width;
	int **map;
	int alive_count;
	int my_alive_count;
	int team_count;
}   t_data;

typedef struct s_agent
{
	int	 agent_id;	   // Unique identifier for this agent
	int	 player;		 // Player id of this agent
	int	 shoot_cooldown; // Number of turns between each of this agent's shots
	int	 optimal_range;  // Maximum manhattan distance for greatest damage output
	int	 soaking_power;  // Damage output within optimal conditions
	int	 splash_bombs;   // Number of splash bombs this can throw this game
	int	 cooldown;	   // Number of turns before this agent can shoot
	int	 x;
	int	 y;
	int	 wetness;		// Damage (0-100) this agent has taken
	bool	alive;
}   t_agent;

/**
 * Win the water fight by controlling the most territory, or out-soak your opponent!
 **/


int	*ft_center(t_data data)
{
	int *center = malloc(sizeof(int) * 2);

	center[0] = data.width / 2;
	center[1] = data.height / 2;
	return (center);
}

double	ft_distance(int x1, int x2, int y1, int y2)
{
	return (sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}

int	*ft_nearest_cover(t_data data, t_agent my_player, int cover_lvl)
{
	int i = -1;
	int j;
	int *spot = malloc(sizeof(int) * 2);
	double dist = data.width * data.height;

	while (++i < data.width)
	{
		j = -1;
		while (++j < data.height)
		{
			if (data.map[i][j] != cover_lvl)
				continue ;
			if (dist > ft_distance(i, my_player.x, j, my_player.y))
			{
				spot[0] = i;
				spot[1] = j;
				dist = ft_distance(i, my_player.x, j, my_player.y);
			}
		}
	}
	return (spot);
}

bool	ft_same_cover(t_data data, t_agent my_player, t_agent *fighters)
{
	int i = my_player.agent_id;

	while (--i > -1)
		if (ft_nearest_cover(data, my_player, 2)[0] == ft_nearest_cover(data, fighters[i], 2)[0]
			&& ft_nearest_cover(data, my_player, 2)[1] == ft_nearest_cover(data, fighters[i], 2)[1])
			return (true);
	return (false);
}

int  ft_nearest_foe(t_data data, t_agent my_player, t_agent *fighters)
{
	int i = -1;
	double dist = data.width * data.height;
	int target = 0;

	while (++i < data.total_count)
	{
		if (fighters[i].player == data.my_id || !fighters[i].alive)
			continue ;
		if (ft_distance(my_player.x, fighters[i].x, my_player.y, fighters[i].y) < dist)
		{
			dist = ft_distance(my_player.x, fighters[i].x, my_player.y, fighters[i].y);
			target = i;
		}
	}
	return (target);
}

bool	ft_shootable(t_data data, t_agent my_player, t_agent *fighters)
{
	int target;

	target = ft_nearest_foe(data, my_player, fighters);
	return (ft_distance(my_player.x, fighters[target].x, my_player.y, fighters[target].y) <= my_player.optimal_range);
}

bool	ft_throwable(t_data data, t_agent my_player, t_agent *fighters)
{
	int target = ft_nearest_foe(data, my_player, fighters);
	int i = -1;

	while (++i < data.total_count)
	{
		if (fighters[i].player != data.my_id)
			continue ;
		if (ft_distance(fighters[i].x, fighters[target].x, fighters[i].y, fighters[target].y) <= 2)
			return (false);
	}
	return (ft_distance(my_player.x, fighters[target].x, my_player.y, fighters[target].y) <= 4 && my_player.splash_bombs);
}

int	ft_count_my_team(t_agent *fighters, int my_id, int total)
{
	int	i = -1;
	int	count = 0;

	while (++i < total)
		if (fighters[i].player == my_id)
			count++;	
	return (count);
}

bool	ft_near_team(t_data data, t_agent *fighters, int i)
{
	int j = i;
	if (data.my_alive_count == 1)
		return (false);
	while (--j > -1 && fighters[j].player == data.my_id)
		if (ft_distance(fighters[j].x, fighters[i].x, fighters[j].y, fighters[i].y) <= 2)
			return (true);
	return (false);
}

int main()
{
	int		turn;
	t_data	data;
	
	turn = 0;
	// Your player id (0 or 1)
	scanf("%d", &data.my_id);
	// Total number of agents in the game
	scanf("%d", &data.total_count);
	t_agent	fighters[data.total_count];

	for (int i = 0; i < data.total_count; i++) {
		scanf("%d%d%d%d%d%d", &fighters[i].agent_id, &fighters[i].player,
			&fighters[i].shoot_cooldown, &fighters[i].optimal_range,
			&fighters[i].soaking_power, &fighters[i].splash_bombs);
			fighters[i].alive = true;
	}

	data.team_count = ft_count_my_team(fighters, data.my_id, data.total_count);
	scanf("%d%d", &data.width, &data.height);
	data.map = malloc(sizeof(int *) * data.width);

	for (int i = 0; i < data.width; i++)
		data.map[i] = malloc(sizeof(int) * data.height);

	int *center = ft_center(data);

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			// X coordinate, 0 is left edge
			int x;
			// Y coordinate, 0 is top edge
			int y;
			int tile_type;
			scanf("%d%d%d", &x, &y, &tile_type);
			data.map[x][y] = tile_type;
		}
	}

	// game loop
	while (1) {
		turn++;
		scanf("%d", &data.alive_count);
		for (int i = 0; i < data.total_count; i++)
			fighters[i].alive = false;
		for (int i = 0; i < data.alive_count; i++)
		{
			int agent_id;
			scanf("%d", &agent_id);
			scanf("%d%d%d%d%d", &fighters[agent_id - 1].x,
				&fighters[agent_id - 1].y, &fighters[agent_id - 1].cooldown,
				&fighters[agent_id - 1].splash_bombs, &fighters[agent_id - 1].wetness);
			fighters[agent_id - 1].alive = true;
		}
		// Number of alive agents controlled by you
		scanf("%d", &data.my_alive_count);
		for (int j = 0; j < data.team_count; j++)
		{
			int	 target;
			int	 i = 0;
			while (fighters[i].player != data.my_id)
				i++;
			i += j;
			int	 *cover2 = ft_nearest_cover(data, fighters[i], 2);
			int	 *cover1 = ft_nearest_cover(data, fighters[i], 1);
			if (!fighters[i].alive)
				continue ;
			target = ft_nearest_foe(data, fighters[i], fighters);
			double  dist_target = ft_distance(fighters[i].x, fighters[target].x, fighters[i].y, fighters[target].y);
	
			// Printing time
			printf("%d", fighters[i].agent_id);
			if (ft_near_team(data, fighters, i) && (turn % 2))
				printf("; MESSAGE LADIES FIRST :D");
			else if (fighters[(i + data.total_count / 2) % data.total_count].alive)
				printf("; MOVE %d %d", fighters[(i + data.total_count / 2) % data.total_count].x, fighters[(i + data.total_count / 2) % data.total_count].y);
			else
				printf("; MOVE %d %d", fighters[target].x, fighters[target].y);
			if (ft_throwable(data, fighters[i], fighters))
				printf("; THROW %d %d", fighters[target].x, fighters[target].y);
			else if (fighters[i].cooldown || !ft_shootable(data, fighters[i], fighters))
				printf("; HUNKER_DOWN");
			else
				printf("; SHOOT %d", fighters[target].agent_id);
			printf("\n");
		}
	}
	return (0);
}


			// else if (!ft_same_cover(data, fighters[i], fighters) && cover2[0])
			//	 printf("; MOVE %d %d", cover2[0] - 1, cover2[1]);
			// else if (cover1[0])
			//	 printf("; MOVE %d %d", cover1[0] - 1, cover1[1]);
			// else if (!data.map[data.width - 1][fighters[i].y])
			//	 printf("; MOVE %d %d", data.width - 1, fighters[i].y);
			// else if (!data.map[data.width - 1][fighters[i].y / 2])
			//	 printf("; MOVE %d %d", data.width - 1, fighters[i].y);



					   // int nearest = ft_nearest_foe(data, fighters[i], fighters);
			// fprintf(stderr, "id: %d nearest foe: %d dist to foe: %.2f\n", fighters[i].agent_id, nearest,
			// ft_distance(fighters[i].x, fighters[nearest].x, fighters[i].y, fighters[nearest].y));
			
			// int *dist = ft_nearest_cover(data, fighters[i], 2);
			// fprintf(stderr, "%d %d dist %.2f\n\n", dist[0], dist[1],
			// ft_distance(fighters[i].x, dist[0], fighters[i].y, dist[1]));
			// One line per agent: <agentId>;<action1;action2;...> actions are "MOVE x y | SHOOT id | THROW x y | HUNKER_DOWN | MESSAGE text"
