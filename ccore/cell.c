#include <assert.h>
#include <math.h>
#include <node_api.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cell.h"
#include "circalloc.h"

static circalloc_t allocator;

void cell_init(napi_env env) {
	napi_status status;

	allocator = circalloc(65536, sizeof(cell_t));

	napi_value symbol;
	status = napi_create_symbol(env, NULL, &symbol);
	assert(status == napi_ok);
}

cell_t* cell_constructor_cell(napi_env env, napi_value cb_this, napi_value world, napi_value x, napi_value y, napi_value size) {
	napi_status status;

	double input_x, input_y, input_size;
	status = napi_get_value_double(env, x, &input_x);
	assert(status == napi_ok);
	status = napi_get_value_double(env, y, &input_y);
	assert(status == napi_ok);
	status = napi_get_value_double(env, size, &input_size);
	assert(status == napi_ok);

	napi_ref obj_ref, world_ref;
	status = napi_create_reference(env, cb_this, 0, &obj_ref);
	assert(status == napi_ok);
	status = napi_create_reference(env, world, 0, &world_ref);
	assert(status == napi_ok);

	napi_value next_cell_id;
	uint32_t next_cell_id_value;
	status = napi_get_named_property(env, world, "nextCellId", &next_cell_id);
	assert(status == napi_ok);
	status = napi_get_value_uint32(env, next_cell_id, &next_cell_id_value);
	assert(status == napi_ok);

	napi_value handle;
	status = napi_get_named_property(env, world, "handle", &handle);
	assert(status == napi_ok);

	napi_value tick;
	int64_t tick_value;
	status = napi_get_named_property(env, handle, "tick", &tick);
	assert(status == napi_ok);
	status = napi_get_value_int64(env, tick, &tick_value);
	assert(status == napi_ok);

	cell_t* cell = circalloc_alloc(&allocator);
	cell->js = obj_ref;
	cell->js_world = world_ref;

	cell->id = next_cell_id_value;
	cell->birth_tick = (uint64_t)tick;
	cell->exists = true;

	cell->x = input_x;
	cell->y = input_y;
	cell->size = input_size;

	cell->color = 0x7f7f7f;
	cell->type = cell_unknown;

	napi_value null_value, false_value, zero_value;
	status = napi_get_null(env, &null_value);
	assert(status == napi_ok);
	status = napi_get_boolean(env, false, &false_value);
	assert(status == napi_ok);
	status = napi_create_double(env, 0.0, &zero_value);
	assert(status == napi_ok);

	napi_value boost_object;
	status = napi_create_object(env, &boost_object);
	assert(status == napi_ok);

	napi_property_descriptor boost_properties[] = {
		{ "dx", 0, 0, 0, 0, zero_value, napi_enumerable | napi_configurable | napi_writable, 0 },
		{ "dy", 0, 0, 0, 0, zero_value, napi_enumerable | napi_configurable | napi_writable, 0 },
		{ "d", 0, 0, 0, 0, zero_value, napi_enumerable | napi_configurable | napi_writable, 0 },
	};

	status = napi_define_properties(env, boost_object, sizeof(boost_properties) / sizeof(napi_property_descriptor), boost_properties);

	napi_property_descriptor properties[] = {
		{ "id", 0, 0, cell_get_id, 0, 0, napi_enumerable, cell },
		{ "exists", 0, 0, cell_get_exists, cell_set_exists, 0, napi_enumerable, cell },
		{ "x", 0, 0, cell_get_x, cell_set_x, 0, napi_enumerable, cell },
		{ "y", 0, 0, cell_get_y, cell_set_y, 0, napi_enumerable, cell },
		{ "size", 0, 0, cell_get_size, cell_set_size, 0, napi_enumerable, cell },
		{ "squareSize", 0, 0, cell_get_square_size, cell_set_square_size, 0, napi_enumerable, cell },
		{ "mass", 0, 0, cell_get_mass, cell_set_mass, 0, napi_enumerable, cell },
		{ "color", 0, 0, cell_get_color, cell_set_color, 0, napi_enumerable, cell },

		{ "eatenBy", 0, 0, 0, 0, null_value, napi_enumerable | napi_configurable | napi_writable, cell },
		{ "range", 0, 0, 0, 0, null_value, napi_enumerable | napi_configurable | napi_writable, cell },
		{ "isBoosting", 0, 0, 0, 0, false_value, napi_enumerable | napi_configurable | napi_writable, cell },
		{ "boost", 0, 0, 0, 0, boost_object, napi_enumerable | napi_configurable | napi_writable, cell },
		{ "owner", 0, 0, 0, 0, null_value, napi_enumerable | napi_configurable | napi_writable, cell },

		{ "name", 0, 0, 0, 0, null_value, napi_enumerable | napi_configurable | napi_writable, cell },
		{ "skin", 0, 0, 0, 0, null_value, napi_enumerable | napi_configurable | napi_writable, cell },

		{ "age", 0, 0, cell_get_age, 0, 0, napi_enumerable, cell },
		{ "shouldUpdate", 0, 0, cell_get_should_update, 0, 0, napi_enumerable, cell },

		{ "onSpawned", 0, cell_on_spawned, 0, 0, 0, napi_enumerable | napi_configurable, cell },
		{ "onTick", 0, cell_on_tick, 0, 0, 0, napi_enumerable | napi_configurable, cell },
		{ "onRemoved", 0, cell_on_removed, 0, 0, 0, napi_enumerable | napi_configurable, cell },
	};

	status = napi_define_properties(env, cb_this, sizeof(properties) / sizeof(napi_property_descriptor), properties);
	assert(status == napi_ok);

	return cell;
}

napi_value virus_constructor(napi_env env, napi_callback_info info) {
	napi_status status;

	size_t argc = 4;
	napi_value argv[4];
	napi_value cb_this;
	status = napi_get_cb_info(env, info, &argc, argv, &cb_this, NULL);
	assert(status == napi_ok);

	cell_t* cell = cell_constructor_cell(env, cb_this, argv[0], argv[1], argv[2], argv[3]);

	cell->color = 0x33ff33;

	cell->type = cell_virus;
	cell_ext_virus_t ext_virus = { 0, 0 };
	cell->ext.virus = ext_virus;

	napi_property_descriptor properties[] = {
		{ "fedTimes", 0, 0, virus_get_fed_times, 0, 0, napi_enumerable, cell },
		{ "splitAngle", 0, 0, virus_get_split_angle, 0, 0, napi_enumerable, cell },

		{ "avoidWhenSpawning", 0, 0, virus_get_avoid_when_spawning, 0, 0, napi_enumerable, cell },
		{ "isAgitated", 0, 0, virus_get_is_agitated, 0, 0, napi_enumerable, cell },
		{ "isSpiked", 0, 0, virus_get_is_spiked, 0, 0, napi_enumerable, cell },
		{ "type", 0, 0, virus_get_type, 0, 0, napi_enumerable, cell },

		{ "getEatResult", 0, virus_eat_result, 0, 0, 0, napi_enumerable, cell },
		{ "onSpawned", 0, virus_on_spawned, 0, 0, 0, napi_enumerable, cell },
		{ "onTick", 0, virus_on_tick, 0, 0, 0, napi_enumerable, cell },
		{ "onRemoved", 0, virus_on_removed, 0, 0, 0, napi_enumerable, cell },
		{ "whenAte", 0, virus_when_ate, 0, 0, 0, napi_enumerable, cell },
		{ "whenEatenBy", 0, virus_when_eaten_by, 0, 0, 0, napi_enumerable, cell },
	};

	status = napi_define_properties(env, cb_this, sizeof(properties) / sizeof(napi_property_descriptor), properties);
	assert(status == napi_ok);

	status = napi_create_reference(env, cb_this, 1, &cell->js);
	assert(status == napi_ok);

	return cb_this;
}

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define CELL_GETTER(fn,cell_location,extra_assert,value_export) \
	napi_value (fn)(napi_env env, napi_callback_info info) { \
		napi_status status; \
		\
		cell_t* cell; \
		status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell); \
		assert(status == napi_ok); \
		\
		(extra_assert);\
		\
		napi_value result; \
		status = (value_export)(env, (cell_location), &result); \
		assert(status == napi_ok); \
		\
		return result; \
	};

#define CELL_SETTER(fn,cell_location,extra_assert,value_import) \
	napi_value (fn)(napi_env env, napi_callback_info info) { \
		napi_status status; \
		\
		cell_t* cell; \
		size_t argc = 1; \
		napi_value argv[1]; \
		status = napi_get_cb_info(env, info, &argc, argv, NULL, (void**)&cell); \
		assert(status == napi_ok); \
		\
		(extra_assert);\
		\
		status = (value_import)(env, argv[0], &(cell_location)); \
		assert(status == napi_ok); \
		\
		return NULL; \
	}

CELL_GETTER(cell_get_id, cell->id, NULL, napi_create_uint32);
CELL_GETTER(cell_get_x, cell->x, NULL, napi_create_double);
CELL_SETTER(cell_set_x, cell->x, NULL, napi_get_value_double);
CELL_GETTER(cell_get_y, cell->y, NULL, napi_create_double);
CELL_SETTER(cell_set_y, cell->y, NULL, napi_get_value_double);
CELL_GETTER(cell_get_size, cell->size, NULL, napi_create_double);
CELL_SETTER(cell_set_size, cell->size, NULL, napi_get_value_double);
CELL_GETTER(cell_get_color, cell->color, NULL, napi_create_uint32);
CELL_SETTER(cell_set_color, cell->color, NULL, napi_get_value_uint32);

napi_value cell_get_exists(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	napi_value result;
	status = napi_get_boolean(env, cell->exists, &result);
	assert(status == napi_ok);

	return result;
}

napi_value cell_set_exists(napi_env env, napi_callback_info info) {
	napi_status status;

	size_t argc = 1;
	napi_value argv[1];
	cell_t* cell;
	status = napi_get_cb_info(env, info, &argc, argv, NULL, (void**)&cell);
	assert(status == napi_ok);

	bool exists;
	status = napi_get_value_bool(env, argv[0], &exists);
	cell->exists = exists;

	return NULL;
}

napi_value cell_get_square_size(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	napi_value result;
	status = napi_create_double(env, cell->size * cell->size, &result);
	assert(status == napi_ok);

	return result;
}

napi_value cell_set_square_size(napi_env env, napi_callback_info info) {
	napi_status status;

	size_t argc = 1;
	napi_value argv[1];
	cell_t* cell;
	status = napi_get_cb_info(env, info, &argc, argv, NULL, (void**)&cell);
	assert(status == napi_ok);

	double square_size;
	status = napi_get_value_double(env, argv[0], &square_size);
	cell->size = sqrt(square_size);

	return NULL;
}

napi_value cell_get_mass(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	napi_value result;
	status = napi_create_double(env, cell->size * cell->size / 100.0, &result);
	assert(status == napi_ok);

	return result;
}

napi_value cell_set_mass(napi_env env, napi_callback_info info) {
	napi_status status;

	size_t argc = 1;
	napi_value argv[1];
	cell_t* cell;
	status = napi_get_cb_info(env, info, &argc, argv, NULL, (void**)&cell);
	assert(status == napi_ok);

	double mass;
	status = napi_get_value_double(env, argv[0], &mass);
	cell->size = sqrt(mass * 100.0);

	return NULL;
}

napi_value cell_get_age(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	napi_value world, handle, tick, step_mult;
	status = napi_get_reference_value(env, cell->js_world, &world);
	assert(status == napi_ok);
	status = napi_get_named_property(env, world, "handle", &handle);
	assert(status == napi_ok);
	status = napi_get_named_property(env, handle, "tick", &tick);
	assert(status == napi_ok);
	status = napi_get_named_property(env, handle, "stepMult", &step_mult);
	assert(status == napi_ok);

	double tick_value, step_mult_value;
	status = napi_get_value_double(env, tick, &tick_value);
	assert(status == napi_ok);
	status = napi_get_value_double(env, step_mult, &step_mult_value);
	assert(status == napi_ok);

	napi_value result;
	status = napi_create_double(env, (tick_value - cell->birth_tick) * step_mult_value, &result);
	assert(status == napi_ok);
	return result;
}

napi_value cell_get_should_update(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_get_boolean(env, false, &result);
	assert(status == napi_ok);
	return result;
}

napi_value cell_on_removed(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	status = napi_reference_unref(env, cell->js, NULL);
	assert(status == napi_ok);

	return NULL;
}

napi_value cell_on_spawned(napi_env env, napi_callback_info info) {
	return NULL;
}

napi_value cell_on_tick(napi_env env, napi_callback_info info) {
	return NULL;
}

CELL_GETTER(virus_get_fed_times, cell->ext.virus.fed_times, assert(cell->type == cell_virus), napi_create_double);
CELL_GETTER(virus_get_split_angle, cell->ext.virus.split_angle, assert(cell->type == cell_virus), napi_create_double);

napi_value virus_get_type(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_create_double(env, 2, &result);
	assert(status == napi_ok);
	return result;
}

napi_value virus_get_is_spiked(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_get_boolean(env, true, &result);
	assert(status == napi_ok);
	return result;
}

napi_value virus_get_is_agitated(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_get_boolean(env, false, &result);
	assert(status == napi_ok);
	return result;
}

napi_value virus_get_avoid_when_spawning(napi_env env, napi_callback_info info) {
	napi_value result;
	napi_status status = napi_get_boolean(env, true, &result);
	assert(status == napi_ok);
	return result;
}

napi_value virus_eat_result(napi_env env, napi_callback_info info) {
	napi_status status;

	cell_t* cell;
	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, info, &argc, argv, NULL, (void**)&cell);
	assert(status == napi_ok);

	napi_value other_type;
	status = napi_get_named_property(env, argv[0], "type", &other_type);
	assert(status == napi_ok);

	double other_type_value;
	status = napi_get_value_double(env, other_type, &other_type_value);
	assert(status == napi_ok);

	double result_value = 0.0;
	if (other_type_value == 3.0) {
		// TODO this.world.virusCount >= this.world.settings.virusMaxCount ? 0 : isSelf ? 2 : 3
		result_value = 2.0;
	} else if (other_type_value == 4.0) {
		result_value = 3.0;
	}

	napi_value result;
	status = napi_create_double(env, result_value, &result);
	assert(status == napi_ok);
	return result;
}

napi_value virus_when_ate(napi_env env, napi_callback_info info) {
	// TODO
	return NULL;
}

napi_value virus_when_eaten_by(napi_env env, napi_callback_info info) {
	// TODO
	return NULL;
}

napi_value virus_on_removed(napi_env env, napi_callback_info info) {
	// this.world.virusCount--
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);

	assert(cell->type == cell_virus);

	napi_value world;
	status = napi_get_reference_value(env, cell->js_world, &world);
	assert(status == napi_ok);

	napi_value virus_count;
	status = napi_get_named_property(env, world, "virusCount", &virus_count);
	assert(status == napi_ok);

	double virus_count_value;
	status = napi_get_value_double(env, virus_count, &virus_count_value);
	assert(status == napi_ok);

	napi_value new_virus_count;
	status = napi_create_double(env, virus_count_value - 1, &new_virus_count);
	assert(status == napi_ok);

	status = napi_set_named_property(env, world, "virusCount", new_virus_count);
	assert(status == napi_ok);

	return cell_on_removed(env, info);
}

napi_value virus_on_spawned(napi_env env, napi_callback_info info) {
	// this.world.virusCount--
	napi_status status;

	cell_t* cell;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&cell);
	assert(status == napi_ok);
	
	assert(cell->type == cell_virus);

	napi_value world;
	status = napi_get_reference_value(env, cell->js_world, &world);
	assert(status == napi_ok);

	napi_value virus_count;
	status = napi_get_named_property(env, world, "virusCount", &virus_count);
	assert(status == napi_ok);

	double virus_count_value;
	status = napi_get_value_double(env, virus_count, &virus_count_value);
	assert(status == napi_ok);

	napi_value new_virus_count;
	status = napi_create_double(env, virus_count_value + 1, &new_virus_count);
	assert(status == napi_ok);

	status = napi_set_named_property(env, world, "virusCount", new_virus_count);
	assert(status == napi_ok);

	return cell_on_spawned(env, info);
}

napi_value virus_on_tick(napi_env env, napi_callback_info info) {
	return cell_on_tick(env, info);
}

#undef CELL_GETTER
#undef CELL_SETTER
