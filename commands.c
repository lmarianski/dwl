#include "commands.h"

// Returns error object, or NULL if check succeeds.
struct cmd_results *checkarg(int argc, const char *name, enum expected_args type, int val) {
	const char *error_name = NULL;
	switch (type) {
	case EXPECTED_AT_LEAST:
		if (argc < val) {
			error_name = "at least ";
		}
		break;
	case EXPECTED_AT_MOST:
		if (argc > val) {
			error_name = "at most ";
		}
		break;
	case EXPECTED_EQUAL_TO:
		if (argc != val) {
			error_name = "";
		}
	}
	return error_name ?
		cmd_results_new(CMD_INVALID, "Invalid %s command "
				"(expected %s%d argument%s, got %d)",
				name, error_name, val, val != 1 ? "s" : "", argc)
		: NULL;
}

/* Keep alphabetized */
static const struct cmd_handler handlers[] = {
	{ "assign", NULL },
	{ "bar", NULL },
	{ "bindcode", NULL },
	{ "bindgesture", NULL },
	{ "bindswitch", NULL },
	{ "bindsym", NULL },
	{ "client.background", NULL },
	{ "client.focused", NULL },
	{ "client.focused_inactive", NULL },
	{ "client.focused_tab_title", NULL },
	{ "client.placeholder", NULL },
	{ "client.unfocused", NULL },
	{ "client.urgent", NULL },
	{ "default_border", NULL },
	{ "default_floating_border", NULL },
	{ "exec", NULL },
	{ "exec_always", NULL },
	{ "floating_maximum_size", NULL },
	{ "floating_minimum_size", NULL },
	{ "floating_modifier", NULL },
	{ "focus", NULL },
	{ "focus_follows_mouse", NULL },
	{ "focus_on_window_activation", NULL },
	{ "focus_wrapping", NULL },
	{ "font", NULL },
	{ "for_window", NULL },
	{ "force_display_urgency_hint", NULL },
	{ "force_focus_wrapping", NULL },
	{ "fullscreen", NULL },
	{ "gaps", NULL },
	{ "hide_edge_borders", NULL },
	{ "input", NULL },
	{ "mode", NULL },
	{ "mouse_warping", NULL },
	{ "new_float", NULL },
	{ "new_window", NULL },
	{ "no_focus", NULL },
	{ "output", NULL },
	{ "popup_during_fullscreen", NULL },
	{ "seat", NULL },
	{ "set", NULL },
	{ "show_marks", NULL },
	{ "smart_borders", NULL },
	{ "smart_gaps", NULL },
	{ "tiling_drag", NULL },
	{ "tiling_drag_threshold", NULL },
	{ "title_align", NULL },
	{ "titlebar_border_thickness", NULL },
	{ "titlebar_padding", NULL },
	{ "unbindcode", NULL },
	{ "unbindgesture", NULL },
	{ "unbindswitch", NULL },
	{ "unbindsym", NULL },
	{ "workspace", cmd_workspace },
	{ "workspace_auto_back_and_forth", NULL },
};

/* Runtime-only commands. Keep alphabetized */
static const struct cmd_handler command_handlers[] = {
	{ "border", NULL },
	{ "create_output", NULL },
	{ "exit", NULL },
	{ "floating", NULL },
	{ "fullscreen", NULL },
	{ "inhibit_idle", NULL },
	{ "kill", NULL },
	{ "layout", NULL },
	{ "mark", NULL },
	{ "max_render_time", NULL },
	{ "move", NULL },
	{ "nop", NULL },
	{ "opacity", NULL },
	{ "reload", NULL },
	{ "rename", NULL },
	{ "resize", NULL },
	{ "scratchpad", NULL },
	{ "shortcuts_inhibitor", NULL },
	{ "split", NULL },
	{ "splith", NULL },
	{ "splitt", NULL },
	{ "splitv", NULL },
	{ "sticky", NULL },
	{ "swap", NULL },
	{ "title_format", NULL },
	{ "unmark", NULL },
	{ "urgent", NULL },
};

static int handler_compare(const void *_a, const void *_b) {
	const struct cmd_handler *a = _a;
	const struct cmd_handler *b = _b;
	return strcasecmp(a->command, b->command);
}

const struct cmd_handler *find_handler(char *line,
		const struct cmd_handler *handlers, size_t handlers_size) {
	const struct cmd_handler query = { .command = line };
	if (!handlers || !handlers_size) {
		return NULL;
	}
	return bsearch(&query, handlers,
			handlers_size / sizeof(struct cmd_handler),
			sizeof(struct cmd_handler), handler_compare);
}

static const struct cmd_handler *find_handler_ex(char *line,
		// const struct cmd_handler *command_handlers, size_t command_handlers_size,
		const struct cmd_handler *handlers, size_t handlers_size) {
	/*
	const struct cmd_handler *handler = NULL;
		handler = find_handler(line, command_handlers, command_handlers_size);
	return handler ? handler : find_handler(line, handlers, handlers_size);
	*/
	return find_handler(line, handlers, handlers_size);
}

static const struct cmd_handler *find_core_handler(char *line) {
	return find_handler_ex(line, 
			//command_handlers, sizeof(command_handlers),
			handlers, sizeof(handlers)
	);
}

char *do_var_replacement(char *str) {
	int i;
	char *find = str;
	while ((find = strchr(find, '$'))) {
		// Skip if escaped.
		if (find > str && find[-1] == '\\') {
			if (find == str + 1 || !(find > str + 1 && find[-2] == '\\')) {
				++find;
				continue;
			}
		}
		// Unescape double $ and move on
		if (find[1] == '$') {
			size_t length = strlen(find + 1);
			memmove(find, find + 1, length);
			find[length] = '\0';
			++find;
			continue;
		}
		// Find matching variable
		/*
		for (i = 0; i < config->symbols->length; ++i) {
			struct sway_variable *var = config->symbols->items[i];
			int vnlen = strlen(var->name);
			if (strncmp(find, var->name, vnlen) == 0) {
				int vvlen = strlen(var->value);
				char *newstr = malloc(strlen(str) - vnlen + vvlen + 1);
				if (!newstr) {
					fprintf(stderr,
						"Unable to allocate replacement "
						"during variable expansion");
					break;
				}
				char *newptr = newstr;
				int offset = find - str;
				strncpy(newptr, str, offset);
				newptr += offset;
				strncpy(newptr, var->value, vvlen);
				newptr += vvlen;
				strcpy(newptr, find + vnlen);
				free(str);
				str = newstr;
				find = str + offset + vvlen;
				break;
			}
		}
		if (i == config->symbols->length) {
			++find;
		}
		*/
	}
	return str;
}

list_t *execute_command(char *_exec, Client *con) {
	char *cmd;
	char *error;
	char matched_delim = ';';
	list_t *containers = NULL;
	bool using_criteria = false;

	char *exec = strdup(_exec);
	char *head = exec;
	list_t *res_list = create_list();
	struct criteria *criteria;

	int i;
	int argc;
	char **argv;
	const struct cmd_handler *handler;
	struct cmd_results *fail_res;
	struct sway_container *container;

	struct cmd_results *res;
	if (!res_list || !exec) {
		return NULL;
	}

	do {
		for (; isspace(*head); ++head) {}
		// Extract criteria (valid for this command list only).
		/*
		if (matched_delim == ';') {
			using_criteria = false;
			if (*head == '[') {
				error = NULL;
				criteria = criteria_parse(head, &error);
				if (!criteria) {
					list_add(res_list,
							cmd_results_new(CMD_INVALID, "%s", error));
					free(error);
					goto cleanup;
				}
				list_free(containers);
				containers = criteria_get_containers(criteria);
				head += strlen(criteria->raw);
				criteria_destroy(criteria);
				using_criteria = true;
				// Skip leading whitespace
				for (; isspace(*head); ++head) {}
			}
		}
		*/
		// Split command list
		cmd = argsep(&head, ";,", &matched_delim);
		for (; isspace(*cmd); ++cmd) {}

		if (strcmp(cmd, "") == 0) {
			fprintf(stderr, "Ignoring empty command.");
			continue;
		}
		fprintf(stderr, "Handling command '%s'", cmd);
		//TODO better handling of argv
		argv = split_args(cmd, &argc);
		if (strcmp(argv[0], "exec") != 0 &&
				strcmp(argv[0], "exec_always") != 0 &&
				strcmp(argv[0], "mode") != 0) {
			for (i = 1; i < argc; ++i) {
				if (*argv[i] == '\"' || *argv[i] == '\'') {
					strip_quotes(argv[i]);
				}
			}
		}
		handler = find_core_handler(argv[0]);
		if (!handler) {
			list_add(res_list, cmd_results_new(CMD_INVALID,
					"Unknown/invalid command '%s'", argv[0]));
			free_argv(argc, argv);
			goto cleanup;
		}

		// Var replacement, for all but first argument of set
		for (i = 1; i < argc; ++i) {
			argv[i] = do_var_replacement(argv[i]);
		}


		/*
		if (!using_criteria) {
			if (con) {
				set_config_node(&con->node, true);
			} else {
				set_config_node(seat_get_focus_inactive(seat, &root->node),
						false);
			}
			res = handler->handle(argc-1, argv+1);
			list_add(res_list, res);
			if (res->status == CMD_INVALID) {
				free_argv(argc, argv);
				goto cleanup;
			}
		} else if (containers->length == 0) {
			list_add(res_list,
					cmd_results_new(CMD_FAILURE, "No matching node."));
		} else {
		*/
			fail_res = NULL;
			res = handler->handle(argc-1, argv+1);
			if (res->status == CMD_SUCCESS) {
				free_cmd_results(res);
			} else {
				// last failure will take precedence
				if (fail_res) {
					free_cmd_results(fail_res);
				}
				fail_res = res;
				if (res->status == CMD_INVALID) {
					list_add(res_list, fail_res);
					free_argv(argc, argv);
					goto cleanup;
				}
			}
			list_add(res_list,
					fail_res ? fail_res : cmd_results_new(CMD_SUCCESS, NULL));
		//}
		free_argv(argc, argv);
	} while(head);
cleanup:
	free(exec);
	list_free(containers);
	return res_list;
}

// this is like execute_command above, except:
// 1) it ignores empty commands (empty lines)
// 2) it does variable substitution
// 3) it doesn't split commands (because the multiple commands are supposed to
//	  be chained together)
// 4) execute_command handles all state internally while config_command has
// some state handled outside (notably the block mode, in read_config)
struct cmd_results *config_command(char *exec, char **new_block) {
	struct cmd_results *results = NULL;
	int argc, i;
	char **argv = split_args(exec, &argc);
	char *temp;
	const struct cmd_handler *handler;
	const char *error;
	char *command;

	// Check for empty lines
	if (!argc) {
		results = cmd_results_new(CMD_SUCCESS, NULL);
		goto cleanup;
	}

	// Check for the start of a block
	if (argc > 1 && strcmp(argv[argc - 1], "{") == 0) {
		*new_block = join_args(argv, argc - 1);
		results = cmd_results_new(CMD_BLOCK, NULL);
		goto cleanup;
	}

	// Check for the end of a block
	if (strcmp(argv[argc - 1], "}") == 0) {
		results = cmd_results_new(CMD_BLOCK_END, NULL);
		goto cleanup;
	}

	// Make sure the command is not stored in a variable
	if (*argv[0] == '$') {
		argv[0] = do_var_replacement(argv[0]);
		temp = join_args(argv, argc);
		free_argv(argc, argv);
		argv = split_args(temp, &argc);
		free(temp);
		if (!argc) {
			results = cmd_results_new(CMD_SUCCESS, NULL);
			goto cleanup;
		}
	}

	// Determine the command handler
	fprintf(stderr, "Config command: %s", exec);
	handler = find_core_handler(argv[0]);
	if (!handler || !handler->handle) {
		error = handler
			? "Command '%s' is shimmed, but unimplemented"
			: "Unknown/invalid command '%s'";
		results = cmd_results_new(CMD_INVALID, error, argv[0]);
		goto cleanup;
	}

	/*
	// Do variable replacement
	if (handler->handle == cmd_set && argc > 1 && *argv[1] == '$') {
		// Escape the variable name so it does not get replaced by one shorter
		char *temp = calloc(1, strlen(argv[1]) + 2);
		temp[0] = '$';
		strcpy(&temp[1], argv[1]);
		free(argv[1]);
		argv[1] = temp;
	}
	*/
	command = do_var_replacement(join_args(argv, argc));
	// fprintf(stderr, "After replacement: %s", command);
	free_argv(argc, argv);
	argv = split_args(command, &argc);
	free(command);

	// Strip quotes and unescape the string
	for (i = 1; i < argc; ++i) {
		/*
		if (handler->handle != cmd_exec && handler->handle != cmd_exec_always
				&& handler->handle != cmd_mode
				&& handler->handle != cmd_bindsym
				&& handler->handle != cmd_bindcode
				&& handler->handle != cmd_bindswitch
				&& handler->handle != cmd_bindgesture
				&& handler->handle != cmd_set
				&& handler->handle != cmd_for_window
				&& (*argv[i] == '\"' || *argv[i] == '\'')) {
				*/
			strip_quotes(argv[i]);
		//}
		unescape_string(argv[i]);
	}

	// Run command
	results = handler->handle(argc - 1, argv + 1);

cleanup:
	free_argv(argc, argv);
	return results;
}

struct cmd_results *config_subcommand(char **argv, int argc,
		const struct cmd_handler *handlers, size_t handlers_size) {
	char *command = join_args(argv, argc);

	const struct cmd_handler *handler = find_handler(argv[0], handlers,
			handlers_size);
	fprintf(stderr, "Subcommand: %s", command);
	free(command);
	if (!handler) {
		return cmd_results_new(CMD_INVALID,
				"Unknown/invalid command '%s'", argv[0]);
	}
	if (handler->handle) {
		return handler->handle(argc - 1, argv + 1);
	}
	return cmd_results_new(CMD_INVALID,
			"The command '%s' is shimmed, but unimplemented", argv[0]);
}

struct cmd_results *config_commands_command(char *exec) {
	struct cmd_results *results = NULL;
	int argc;
	char **argv = split_args(exec, &argc);
	char *cmd = argv[0];
	const struct cmd_handler *handler;
	if (!argc) {
		results = cmd_results_new(CMD_SUCCESS, NULL);
		goto cleanup;
	}

	// Find handler for the command this is setting a policy for

	if (strcmp(cmd, "}") == 0) {
		results = cmd_results_new(CMD_BLOCK_END, NULL);
		goto cleanup;
	}

	handler = find_handler(cmd, NULL, 0);
	if (!handler && strcmp(cmd, "*") != 0) {
		results = cmd_results_new(CMD_INVALID,
			"Unknown/invalid command '%s'", cmd);
		goto cleanup;
	}

	results = cmd_results_new(CMD_SUCCESS, NULL);

cleanup:
	free_argv(argc, argv);
	return results;
}

struct cmd_results *cmd_results_new(enum cmd_status status,
		const char *format, ...) {
	struct cmd_results *results = malloc(sizeof(struct cmd_results));
	if (!results) {
		fprintf(stderr, "Unable to allocate command results");
		return NULL;
	}
	results->status = status;
	if (format) {
		char *error = malloc(256);
		va_list args;
		va_start(args, format);
		if (error) {
			vsnprintf(error, 256, format, args);
		}
		va_end(args);
		results->error = error;
	} else {
		results->error = NULL;
	}
	return results;
}

void free_cmd_results(struct cmd_results *results) {
	if (results->error) {
		free(results->error);
	}
	free(results);
}

char *cmd_results_to_json(list_t *res_list) {
	const char *json;
	char *res;
	json_object *result_array = json_object_new_array();
	for (int i = 0; i < res_list->length; ++i) {
		struct cmd_results *results = res_list->items[i];
		json_object *root = json_object_new_object();
		json_object_object_add(root, "success",
				json_object_new_boolean(results->status == CMD_SUCCESS));
		if (results->error) {
			json_object_object_add(root, "parse_error",
					json_object_new_boolean(results->status == CMD_INVALID));
			json_object_object_add(
					root, "error", json_object_new_string(results->error));
		}
		json_object_array_add(result_array, root);
	}
	json = json_object_to_json_string(result_array);
	res = strdup(json);
	json_object_put(result_array);
	return res;
}
