#include <sys/printk.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "board.h"
#include "main.h"
#include "onoff_cli.h"

/* Model Operation Codes */
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET		BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET		BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS	BT_MESH_MODEL_OP_2(0x82, 0x04)

static void gen_onoff_status(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf);

const struct bt_mesh_model_op gen_onoff_cli_op[] = {
	{ BT_MESH_MODEL_OP_GEN_ONOFF_STATUS, 1, gen_onoff_status },
	BT_MESH_MODEL_OP_END,
};

struct button btn_a = {
	.model = &root_models[2],
};

static void mb_on_new_state(u8_t state)
{
	if(state)
		light_on();
	else
		light_off();
}

struct onoff_state onoff_cli_state = {
	.current = 0,
	.on_new_state = mb_on_new_state
};

static u8_t tid = 0;

static void gen_onoff_set_unack(struct bt_mesh_model *model)
{
	struct net_buf_simple *msg = model->pub->msg;
	struct onoff_state *cli_state = model->user_data;
	u8_t new_state;
	int err;

	new_state = cli_state->current ? 0 : 1;
  printk("Previous state is: %d, Publish new state: %d\n", cli_state->current, new_state);

	// Generic OnOff Set Unacknowledged
	bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK);
	net_buf_simple_add_u8(msg, new_state);
  net_buf_simple_add_u8(msg, tid++);
	err = bt_mesh_model_publish(model);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
		return;
	}

	cli_state->current = new_state;
	cli_state->on_new_state(new_state);
}

static void gen_onoff_get(struct bt_mesh_model *model)
{
	struct net_buf_simple *msg = model->pub->msg;
	int err;

	bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_GET);

	err = bt_mesh_model_publish(model);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
	}
}

static void gen_onoff_set(struct bt_mesh_model *model)
{
	gen_onoff_set_unack(model);
	gen_onoff_get(model);
}

static void gen_onoff_status(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
	u8_t state = net_buf_simple_pull_u8(buf);
	printk("Onoff client model of element(%04x) recived new state"
		"from: %04x with state: %d\n",
		bt_mesh_model_elem(model)->addr, ctx->addr, state);
}

static void button_a_pressed_worker(struct k_work *work)
{
	struct button *btn = CONTAINER_OF(work, struct button, work);

	if(primary_addr == BT_MESH_ADDR_UNASSIGNED)
		return;

	gen_onoff_set(btn->model);

}

void onoff_cli_init()
{
  /* Initialize button worker task*/
	k_work_init(&btn_a.work, button_a_pressed_worker);
}