/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "visualconfig.h"
/* host */
#include "dirconfig.h"
#include "skinio.h"
#include "styles.h"
#include "resources/resource.h"
/* ui */
#include <uicomponent.h> /* Translator */
#include <uiapp.h> /* Styles */
#include <uiopendialog.h>
/* audio */
#include <player.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__OS__POSIX
#define _MAX_PATH 4096
#endif

static void visualconfig_make(VisualConfig*, psy_Property*);
static void visualconfig_load_control_skin(VisualConfig* self);
static void visualconfig_on_set_app_theme(VisualConfig*, psy_Property* sender);
static void visualconfig_on_reset_skin(VisualConfig*, psy_Property* sender);
static void visualconfig_on_load_skin(VisualConfig*, psy_Property* sender);
static void visualconfig_on_reset_control_skin(VisualConfig*, psy_Property* sender);
static void visualconfig_on_load_control_skin(VisualConfig*, psy_Property* sender);
static void visualconfig_on_draw_vu_meters(VisualConfig*, psy_Property* sender);
static void visualconfig_on_set_default_font(VisualConfig*, psy_Property* sender);

void visualconfig_init(VisualConfig* self, psy_Property* parent,
	psy_audio_Player* player)
{
	assert(self);
	assert(parent);
	assert(player);

	self->parent = parent;
	self->dirconfig = NULL;
	self->player = player;	
	visualconfig_make(self, parent);
}

void visualconfig_dispose(VisualConfig* self)
{
	assert(self);

	patternviewconfig_dispose(&self->patview);
	machineviewconfig_dispose(&self->macview);
	machineparamconfig_dispose(&self->macparam);	
}

void visualconfig_make(VisualConfig* self, psy_Property* parent)
{
	assert(self);

	self->visual = psy_property_set_text(
		psy_property_append_section(parent, "visual"),
		"settingsview.visual.visual");
	psy_property_set_icon(self->visual, IDB_IMAGE_LIGHT,
		IDB_IMAGE_DARK);
	psy_property_connect(psy_property_set_id(
		psy_property_set_text(psy_property_append_action(
			self->visual, "loadskin"),
			"settingsview.visual.load-skin"), PROPERTY_ID_LOADSKIN),
		self, visualconfig_on_load_skin);
	psy_property_connect(psy_property_set_id(
		psy_property_set_text(
			psy_property_append_action(self->visual, "defaultskin"),
			"settingsview.visual.default-skin"),
		PROPERTY_ID_DEFAULTSKIN), self, visualconfig_on_reset_skin);
	self->defaultfont = psy_property_connect(psy_property_set_id(psy_property_set_text(
		psy_property_append_font(self->visual, "defaultfont",
			PSYCLE_DEFAULT_FONT),
		"settingsview.visual.default-font"),
		PROPERTY_ID_DEFAULTFONT),
		self, visualconfig_on_set_default_font);
	psy_property_set_hint(psy_property_set_text(psy_property_append_double(
		self->visual, "zoom", 1.0, 0.1, 4.0),
		"settingsview.visual.zoom"),
		PSY_PROPERTY_HINT_ZOOM);
	self->apptheme =
		psy_property_connect(psy_property_set_id(psy_property_set_text(
			psy_property_append_choice(self->visual,
				"apptheme", 1),
			"settingsview.visual.apptheme"),
			PROPERTY_ID_APPTHEME),
			self, visualconfig_on_set_app_theme);			
	psy_property_set_text(
		psy_property_append_int(self->apptheme, "light", psy_ui_LIGHTTHEME, 0, 2),
		"settingsview.visual.light");
	psy_property_set_text(
		psy_property_append_int(self->apptheme, "dark", psy_ui_DARKTHEME, 0, 2),
		"settingsview.visual.dark");
	psy_property_set_text(
		psy_property_append_int(self->apptheme, "win98", psy_ui_WIN98THEME, 0, 2),
		"Windows 98");
	patternviewconfig_init(&self->patview, self->visual,
		PSYCLE_SKINS_DEFAULT_DIR);
	machineviewconfig_init(&self->macview, self->visual);
	machineparamconfig_init(&self->macparam, self->visual);
}

void visualconfig_set_directories(VisualConfig* self, DirConfig*
	dirconfig)
{
	assert(self);

	self->dirconfig = dirconfig;
	patternviewtheme_set_directories(&self->patview.theme, self->dirconfig);
	machineparamconfig_setdirectories(&self->macparam, self->dirconfig);
	machineviewconfig_setdirectories(&self->macview, self->dirconfig);
}

void visualconfig_load_skin(VisualConfig* self, const char* path)
{
	psy_Property skin;
	const char* machine_gui_bitmap;

	assert(self);

	if (!self->dirconfig) {
		return;
	}
	psy_property_init(&skin);
	skin_load(&skin, path);
	machine_gui_bitmap = psy_property_at_str(&skin, "machine_GUI_bitmap", 0);
	if (machine_gui_bitmap) {
		char psc[_MAX_PATH];

		psy_dir_findfile(dirconfig_skins(self->dirconfig),
			machine_gui_bitmap, psc);
		if (psc[0] != '\0') {
			psy_Path path;

			psy_path_init(&path, psc);
			if (strcmp(psy_path_ext(&path), "bmp") == 0) {
				psy_property_set_str(&skin, "machinedial_bmp",
					psy_path_full(&path));
			}
			else if (skin_load_psc(&skin, psc) == PSY_OK) {
				const char* bpm;

				bpm = psy_property_at_str(&skin, "machinedial_bmp", NULL);
				if (bpm) {
					psy_Path full;

					psy_path_init(&full, psc);
					psy_path_set_name(&full, "");
					psy_path_setext(&full, "");
					psy_path_set_name(&full, bpm);
					psy_property_set_str(&skin, "machinedial_bmp",
						psy_path_full(&full));
					psy_path_dispose(&full);
				}
			}
		}
	}
	machineparamconfig_settheme(&self->macparam, &skin);
	machineviewconfig_settheme(&self->macview, &skin);
	patternviewconfig_set_theme(&self->patview, &skin);
	psy_property_dispose(&skin);
}


void visualconfig_reset_skin(VisualConfig* self)
{
	assert(self);

	patternviewconfig_reset_theme(&self->patview);
	machineviewconfig_resettheme(&self->macview);
	machineparamconfig_resettheme(&self->macparam);
}

void visualconfig_reset_control_skin(VisualConfig* self)
{
	assert(self);

	machineparamconfig_resettheme(&self->macparam);
}

const char* visualconfig_default_font_str(const VisualConfig* self)
{
	assert(self);

	return psy_property_at_str(self->visual, "defaultfont",
		PSYCLE_DEFAULT_FONT);
}

void visualconfig_set_default_font(VisualConfig* self, psy_Property* property)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_FONT) {
		psy_ui_Font font;
		psy_ui_FontInfo fontinfo;

		psy_ui_fontinfo_init_string(&fontinfo, psy_property_item_str(property));
		psy_ui_font_init(&font, &fontinfo);
		psy_ui_app_set_default_font(psy_ui_app(), &font);
		psy_ui_font_dispose(&font);
	}
}

bool visualconfig_hasproperty(const VisualConfig* self, psy_Property* property)
{
	assert(self);

	assert(self->visual);

	return psy_property_in_section(property, self->visual);
}

void visualconfig_on_load_skin(VisualConfig* self, psy_Property* sender)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Theme", "Psycle Display Presets|*.psv", "PSV",
		dirconfig_skins(self->dirconfig));
	if (psy_ui_opendialog_execute(&opendialog)) {
		visualconfig_load_skin(self, psy_path_full(psy_ui_opendialog_path(
			&opendialog)));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void visualconfig_load_control_skin(VisualConfig* self)
{
	psy_ui_OpenDialog opendialog;

	psy_ui_opendialog_init_all(&opendialog, 0,
		"Load Dial Bitmap",
		"Control Skins|*.psc|Bitmaps|*.bmp", "psc",
		dirconfig_skins(self->dirconfig));
	if (psy_ui_opendialog_execute(&opendialog)) {
		machineparamconfig_set_dial_bpm(&self->macparam,
			psy_path_full(psy_ui_opendialog_path(&opendialog)));
	}
	psy_ui_opendialog_dispose(&opendialog);
}

void visualconfig_on_set_app_theme(VisualConfig* self, psy_Property* sender)
{
	visualconfig_set_app_theme(self);
}

void visualconfig_set_app_theme(VisualConfig* self)
{
	psy_Property* choice;

	assert(self);

	choice = psy_property_at_choice(self->apptheme);
	if (choice) {
		psy_ui_ThemeMode theme;

		theme = (psy_ui_ThemeMode)(psy_property_item_int(choice));
		/* reset styles */
		psy_ui_defaults_inittheme(psy_ui_appdefaults(), theme, TRUE);
		init_host_styles(&psy_ui_appdefaults()->styles, theme);
		machineviewconfig_load(&self->macview);
		machineparamconfig_update_styles(&self->macparam);
		patternviewtheme_write_styles(&self->patview.theme);
		psy_ui_defaults_load_theme(psy_ui_appdefaults(),
			dirconfig_config_dir(self->dirconfig), theme);
		if (psy_ui_app()->imp) {
			psy_ui_app()->imp->vtable->dev_onappdefaultschange(
				psy_ui_app()->imp);
		}
		psy_ui_appzoom_update_base_fontsize(&psy_ui_app()->zoom,
			psy_ui_defaults_font(&psy_ui_app()->defaults));
		psy_ui_notify_style_update(psy_ui_app()->main);
	}
}

void visualconfig_on_reset_skin(VisualConfig* self, psy_Property* sender)
{
	visualconfig_reset_skin(self);
}
	
void visualconfig_on_reset_control_skin(VisualConfig* self, psy_Property* sender)
{
	visualconfig_reset_control_skin(self);
}

void visualconfig_on_load_control_skin(VisualConfig* self, psy_Property* sender)
{
	visualconfig_load_control_skin(self);
}

void visualconfig_on_draw_vu_meters(VisualConfig* self, psy_Property* sender)
{
	if (psy_property_item_bool(sender)) {
		psy_audio_player_set_vu_meter_mode(self->player, VUMETER_RMS);
	}
	else {
		psy_audio_player_set_vu_meter_mode(self->player, VUMETER_NONE);
	}
}

void visualconfig_on_set_default_font(VisualConfig* self, psy_Property* sender)
{
	visualconfig_set_default_font(self, sender);
}
