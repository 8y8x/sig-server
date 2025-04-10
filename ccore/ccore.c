#include <assert.h>
#include <node_api.h>
#include <stdlib.h>

#include "cell.h"

static napi_value init(napi_env env, napi_value exports) {
	cell_init(env);

	napi_status status;

	napi_property_descriptor properties[] = {
		{ "Virus", 0, virus_constructor, 0, 0, 0, napi_enumerable, 0 },
	};

	status = napi_define_properties(env, exports, sizeof(properties) / sizeof(napi_property_descriptor), properties);
	assert(status == napi_ok);
	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
