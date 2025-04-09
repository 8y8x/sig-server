#include <node_api.h>
#include <stdbool.h>

typedef enum cell_type {
	cell_virus = 2,
	cell_unknown
} cell_type;

typedef struct cell_ext_virus_t {
	double fed_times;
	double split_angle;
} cell_ext_virus_t;

typedef struct cell_t {
	napi_ref js;
	napi_ref js_world;

	uint32_t id;
	uint64_t birth_tick;
	bool exists;

	double x;
	double y;
	double size;

	uint32_t color;

	cell_type type;
	union {
		cell_ext_virus_t virus;
	} ext;
} cell_t;

#define NODE_CALLBACK(name) napi_value (name)(napi_env env, napi_callback_info info);

void cell_init(napi_env env);
cell_t* cell_constructor_cell(napi_env env, napi_value cb_this, napi_value world, napi_value x, napi_value y, napi_value size);

NODE_CALLBACK(cell_get_id);
NODE_CALLBACK(cell_get_exists);
NODE_CALLBACK(cell_set_exists);
NODE_CALLBACK(cell_get_x);
NODE_CALLBACK(cell_set_x);
NODE_CALLBACK(cell_get_y);
NODE_CALLBACK(cell_set_y);
NODE_CALLBACK(cell_get_size);
NODE_CALLBACK(cell_set_size);
NODE_CALLBACK(cell_get_square_size);
NODE_CALLBACK(cell_set_square_size);
NODE_CALLBACK(cell_get_mass);
NODE_CALLBACK(cell_set_mass);
NODE_CALLBACK(cell_get_color);
NODE_CALLBACK(cell_set_color);

NODE_CALLBACK(cell_get_age);
NODE_CALLBACK(cell_get_should_update);

NODE_CALLBACK(cell_on_removed);
NODE_CALLBACK(cell_on_spawned);
NODE_CALLBACK(cell_on_tick);

NODE_CALLBACK(virus_constructor);
NODE_CALLBACK(virus_get_avoid_when_spawning);
NODE_CALLBACK(virus_get_fed_times);
NODE_CALLBACK(virus_get_is_agitated);
NODE_CALLBACK(virus_get_is_spiked);
NODE_CALLBACK(virus_get_split_angle);
NODE_CALLBACK(virus_get_type);

NODE_CALLBACK(virus_eat_result);
NODE_CALLBACK(virus_on_removed);
NODE_CALLBACK(virus_on_spawned);
NODE_CALLBACK(virus_on_tick);
NODE_CALLBACK(virus_when_ate);
NODE_CALLBACK(virus_when_eaten_by);
