#ifndef _ONOFF_CLI_H
#define _ONOFF_CLI_H

struct button {
	struct bt_mesh_model *model;
	struct k_work work;
};

struct onoff_state {
	u8_t current;
	void (*on_new_state)(u8_t state);
};

extern struct button btn_a;
extern struct onoff_state onoff_cli_state;

extern const struct bt_mesh_model_op gen_onoff_cli_op[];


extern void onoff_cli_init();



#endif