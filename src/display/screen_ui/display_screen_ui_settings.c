#include "display_screen_ui.h"

#include "global/global_strings.h"

#define UI_NAME settings

static struct {
	lv_obj_t* screen;
	lv_obj_t* window;
	lv_obj_t* window_done_button;
	lv_obj_t* general;
	lv_obj_t* brightness_label;
	lv_obj_t* brightness;
	lv_obj_t* nfc;
	lv_obj_t* nfc_nyi;
	lv_obj_t* cryptography;
	lv_obj_t* cryptography_key_status_label;
	lv_obj_t* cryptography_select_keys_button;
	lv_obj_t* cryptography_clear_keys_button;
	lv_obj_t* bootloader;
	lv_obj_t* bootloader_text;
	lv_obj_t* bootloader_button;
	lv_group_t* active_group;
} ui;

static lv_style_t style_box;

static ui_common_simple_cb done_cb = NULL;
static ui_common_float_cb brightness_cb = NULL;
static ui_common_simple_cb select_crypto_keys_cb = NULL;
static ui_common_simple_cb clear_crypto_keys_cb = NULL;
static ui_common_simple_cb enter_dfu_cb = NULL;

static void settings_done_pressed(lv_obj_t* obj, lv_event_t e);
static void brightness_slider_event_cb(lv_obj_t* slider, lv_event_t e);
static void settings_focus_cb(lv_group_t* group);

UI_DEFINE_SIMPLE_BUTTON_PRESS_CALLBACK_HANDLER(settings_select_cryptography_keys_pressed, select_crypto_keys_cb)
UI_DEFINE_SIMPLE_BUTTON_PRESS_CALLBACK_HANDLER(settings_clear_cryptography_keys_pressed, select_crypto_keys_cb)
UI_DEFINE_SIMPLE_BUTTON_PRESS_CALLBACK_HANDLER(settings_enter_dfu_pressed, enter_dfu_cb)

static const char* cryptography_key_status_prefix = "Key status: ";

UI_DECLARE_CREATE(UI_NAME)
{
	if (ui.screen != NULL)
	{
		done_cb = NULL;
		brightness_cb = NULL;
		return ui.screen;
	}

    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

	ui.screen = lv_obj_create(NULL, NULL);

	// window
	ui.window = lv_win_create(ui.screen, NULL);

	// 'done' (back) button
	ui.window_done_button = lv_win_add_btn_left(ui.window, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(ui.window_done_button, settings_done_pressed);
	ui_common_set_window_button_style(ui.window_done_button);

	// header
	lv_win_set_title(ui.window, "Settings");

	// the page content
	lv_obj_t* content = lv_win_get_content(ui.window);
	lv_page_set_scrl_layout(content, LV_LAYOUT_PRETTY_TOP);
    //lv_cont_set_fit2(content, LV_FIT_NONE, LV_FIT_TIGHT);

	// general container
	ui.general = lv_cont_create(content, NULL);
    lv_cont_set_layout(ui.general, LV_LAYOUT_COLUMN_LEFT);
    lv_obj_add_style(ui.general, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(ui.general, true);
    lv_cont_set_fit2(ui.general, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width_fit(ui.general, lv_obj_get_width(content));
    lv_obj_set_style_local_value_str(ui.general, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "General");

    lv_coord_t fit_w = lv_obj_get_width_fit(ui.general);

	// brightness setting label
	ui.brightness_label = lv_label_create(ui.general, NULL);
	ui_common_set_label_font_theme_small(ui.brightness_label);
	lv_label_set_align(ui.brightness_label, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_width_margin(ui.brightness_label, fit_w);
	lv_label_set_text(ui.brightness_label, "Backlight");

	// brightness slider
	ui.brightness = lv_slider_create(ui.general, NULL);
	lv_slider_set_range(ui.brightness, 0, 100);
    lv_slider_set_value(ui.brightness, 100, LV_ANIM_OFF);
    lv_obj_set_event_cb(ui.brightness, brightness_slider_event_cb);
    lv_obj_set_width_margin(ui.brightness, fit_w);

	// make the slider fancy
    lv_obj_set_style_local_margin_top(ui.brightness, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(ui.brightness, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

	// nfc container
	ui.nfc = lv_cont_create(content, ui.general);
    lv_obj_set_style_local_value_str(ui.nfc, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "NFC");
	lv_obj_set_event_cb(ui.nfc, ui_common_up_down_focus_cb);

	// nfc coming soon
	ui.nfc_nyi = lv_label_create(ui.nfc, ui.brightness_label);
	lv_label_set_text(ui.nfc_nyi, "NFC settings are not yet implemented.\n"
		"They will come in a future update.");

	// cryptography container
	ui.cryptography = lv_cont_create(content, ui.general);
    lv_obj_set_style_local_value_str(ui.cryptography, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Cryptography");
	lv_obj_set_event_cb(ui.cryptography, ui_common_up_down_focus_cb);

	// aes key status label
	ui.cryptography_key_status_label = lv_label_create(ui.cryptography, ui.brightness_label);
	lv_label_set_text(ui.cryptography_key_status_label, cryptography_key_status_prefix);

	// select aes key(s) button
	ui.cryptography_select_keys_button = lv_btn_create(ui.cryptography, NULL);
	lv_obj_set_event_cb(ui.cryptography_select_keys_button, settings_select_cryptography_keys_pressed);
	lv_obj_set_width_fit(ui.cryptography_select_keys_button, fit_w / 2);
	lv_obj_set_style_local_border_color(ui.cryptography_select_keys_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_PURPLE);
	lv_obj_set_style_local_outline_color(ui.cryptography_select_keys_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_RED);
	lv_obj_t* cryptography_select_keys_button_label = lv_label_create(ui.cryptography_select_keys_button, NULL);
	lv_label_set_text(cryptography_select_keys_button_label, "Select Key(s)");

	// clear aes key(s) button
	ui.cryptography_clear_keys_button = lv_btn_create(ui.cryptography, NULL);
	lv_obj_set_event_cb(ui.cryptography_clear_keys_button, settings_clear_cryptography_keys_pressed);
	lv_obj_set_width_fit(ui.cryptography_clear_keys_button, fit_w / 2);
	lv_obj_set_style_local_border_color(ui.cryptography_clear_keys_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_PURPLE);
	lv_obj_set_style_local_outline_color(ui.cryptography_clear_keys_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_RED);
	lv_obj_t* cryptography_clear_keys_button_label = lv_label_create(ui.cryptography_clear_keys_button, NULL);
	lv_label_set_text(cryptography_clear_keys_button_label, "Clear Key(s)");

	// bootloader container
	ui.bootloader = lv_cont_create(content, ui.general);
    lv_obj_set_style_local_value_str(ui.bootloader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Firmware Update");
	lv_obj_set_event_cb(ui.bootloader, ui_common_up_down_focus_cb);

	// bootloader help text
	ui.bootloader_text = lv_label_create(ui.bootloader, ui.brightness_label);
	lv_label_set_text(ui.bootloader_text,
		"Press the button below to restart " PRODUCT_NAME_SHORT "\n"
		"in firmware (DFU) update mode. After the screen\n"
		"goes off, attach " PRODUCT_NAME_SHORT " to your PC, and look\n"
		"for a USB Disk named " PRODUCT_NAME_SHORT "BOOT. Drag the\n"
		"UF2 file to it to flash new firmware. If\n"
		"this process isn't completed in 5 minutes, \n"
		PRODUCT_NAME_SHORT " will reboot.\n"
		"\n"
		"Current version:\n"
		PRODUCT_NAME_FULL_BUILD);

	ui.bootloader_button = lv_btn_create(ui.bootloader, NULL);
	lv_obj_set_event_cb(ui.bootloader_button, settings_enter_dfu_pressed);
	lv_obj_set_width_fit(ui.bootloader_button, fit_w);
	lv_obj_set_style_local_border_color(ui.bootloader_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_PURPLE);
	lv_obj_set_style_local_outline_color(ui.bootloader_button, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_RED);
    lv_obj_t* bootloader_button_label = lv_label_create(ui.bootloader_button, NULL);
    lv_label_set_text(bootloader_button_label, "Reboot to DFU Mode");

	return ui.screen;
}

UI_DECLARE_ACTIVATE(UI_NAME)
{
	lv_scr_load(ui.screen);
	lv_group_set_focus_cb(group, settings_focus_cb);
	lv_group_remove_all_objs(group);

	lv_group_add_obj(group, ui.window_done_button);

	lv_group_add_obj(group, ui.brightness);

	lv_group_add_obj(group, ui.cryptography);
	lv_group_add_obj(group, ui.cryptography_select_keys_button);
	lv_group_add_obj(group, ui.cryptography_clear_keys_button);

	lv_group_add_obj(group, ui.nfc);

	lv_group_add_obj(group, ui.bootloader);
	lv_group_add_obj(group, ui.bootloader_button);

	lv_group_focus_obj(ui.brightness);

	ui_common_cb_set_active_group(group);

	ui.active_group = group;

	// add status widget
	ui_status_widget_get(ui.screen);
}

void UI_DECLARE_FUNCTION(UI_NAME, set_done_callback)(ui_common_simple_cb cb)
{
	done_cb = cb;
}

void UI_DECLARE_FUNCTION(UI_NAME, set_brightness_callback)(ui_common_float_cb cb)
{
	brightness_cb = cb;
}

void UI_DECLARE_FUNCTION(UI_NAME, set_select_crypto_keys_callback)(ui_common_simple_cb cb)
{
	select_crypto_keys_cb = cb;
}

void UI_DECLARE_FUNCTION(UI_NAME, set_clear_crypto_keys_callback)(ui_common_simple_cb cb)
{
	clear_crypto_keys_cb = cb;
}

void UI_DECLARE_FUNCTION(UI_NAME, set_enter_dfu_callback)(ui_common_simple_cb cb)
{
	enter_dfu_cb = cb;
}

static void settings_done_pressed(lv_obj_t* obj, lv_event_t e)
{
	if (e == LV_EVENT_CLICKED)
	{
		if (done_cb != NULL)
		{
			done_cb();
		}
		else
		{
			// they aren't handling cancel,
			// we will delete our window
			// so we can be created again in the future.
			lv_win_close_event_cb(obj, e);
			lv_obj_del(ui.screen);
			ui.screen = NULL;
		}
	}
	else
	{
		ui_common_up_down_focus_cb(obj, e);
	}
}

static void brightness_slider_event_cb(lv_obj_t* slider, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED && lv_obj_is_focused(slider))
	{
		uint16_t level_int = lv_slider_get_value(slider);
		float level = level_int / 100.0f;

		// tell someone it's changed!
		if (brightness_cb != NULL)
		{
			brightness_cb(level);
		}

		static char buf[16];
		lv_snprintf(buf, sizeof(buf), "%d", lv_slider_get_value(slider));
		lv_obj_set_style_local_value_str(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
    }
	else
	{
		ui_common_up_down_focus_cb(slider, e);
	}
}

static void settings_focus_cb(lv_group_t* group)
{
	lv_win_focus(ui.window, *group->obj_focus, LV_ANIM_ON);
}
