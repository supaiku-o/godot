/*************************************************************************/
/*  editor_run_native.cpp                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "editor_run_native.h"

#include "editor_export.h"
#include "editor_node.h"

void EditorRunNative::_notification(int p_what) {

	if (p_what == NOTIFICATION_ENTER_TREE) {

		for (int i = 0; i < EditorExport::get_singleton()->get_export_platform_count(); i++) {

			Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(i);
			if (eep.is_null())
				continue;
			Ref<ImageTexture> icon = eep->get_logo();
			if (!icon.is_null()) {
				Image im = icon->get_data();
				im.clear_mipmaps();
				if (!im.empty()) {

					im.resize(16, 16);
					Ref<ImageTexture> small_icon;
					small_icon.instance();
					small_icon->create_from_image(im, 0);
					MenuButton *mb = memnew(MenuButton);
					mb->get_popup()->connect("id_pressed", this, "_run_native", varray(i));
					//mb->connect("pressed", this, "_run_native", varray(-1, i));
					mb->set_icon(small_icon);
					add_child(mb);
					menus[i] = mb;
				}
			}
		}
	}

	if (p_what == NOTIFICATION_PROCESS) {

		bool changed = EditorExport::get_singleton()->poll_export_platforms() || first;

		if (changed) {

			for (Map<int, MenuButton *>::Element *E = menus.front(); E; E = E->next()) {

				Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(E->key());
				MenuButton *mb = E->get();
				int dc = eep->get_device_count();

				if (dc == 0) {
					mb->hide();
				} else {
					mb->get_popup()->clear();
					mb->show();
					mb->set_tooltip("Select device from the list");
					for (int i = 0; i < dc; i++) {
						mb->get_popup()->add_icon_item(get_icon("Play", "EditorIcons"), eep->get_device_name(i));
						mb->get_popup()->set_item_tooltip(mb->get_popup()->get_item_count() - 1, eep->get_device_info(i).strip_edges());
					}
				}
			}

			first = false;
		}
	}
}

void EditorRunNative::_run_native(int p_idx, int p_platform) {

	Ref<EditorExportPlatform> eep = EditorExport::get_singleton()->get_export_platform(p_platform);
	ERR_FAIL_COND(eep.is_null());
	/*if (p_idx == -1) {
		if (eep->get_device_count() == 1) {
			menus[p_platform]->get_popup()->hide();
			p_idx = 0;
		} else {
			return;
		}
	}*/

	Ref<EditorExportPreset> preset;

	for (int i = 0; i < EditorExport::get_singleton()->get_export_preset_count(); i++) {

		Ref<EditorExportPreset> ep = EditorExport::get_singleton()->get_export_preset(i);
		if (ep->is_runnable() && ep->get_platform() == eep) {
			preset = ep;
			break;
		}
	}

	if (preset.is_null()) {
		EditorNode::get_singleton()->show_warning("No runnable export preset found for this platform.\nPlease add a runnable preset in the export menu.");
		return;
	}

	emit_signal("native_run");

	int flags = 0;
	if (deploy_debug_remote)
		flags |= EditorExportPlatform::DEBUG_FLAG_REMOTE_DEBUG;
	if (deploy_dumb)
		flags |= EditorExportPlatform::DEBUG_FLAG_DUMB_CLIENT;
	if (debug_collisions)
		flags |= EditorExportPlatform::DEBUG_FLAG_VIEW_COLLISONS;
	if (debug_navigation)
		flags |= EditorExportPlatform::DEBUG_FLAG_VIEW_NAVIGATION;

	eep->run(preset, p_idx, flags);
}

void EditorRunNative::_bind_methods() {

	ClassDB::bind_method("_run_native", &EditorRunNative::_run_native);

	ADD_SIGNAL(MethodInfo("native_run"));
}

void EditorRunNative::set_deploy_dumb(bool p_enabled) {

	deploy_dumb = p_enabled;
}

bool EditorRunNative::is_deploy_dumb_enabled() const {

	return deploy_dumb;
}

void EditorRunNative::set_deploy_debug_remote(bool p_enabled) {

	deploy_debug_remote = p_enabled;
}

bool EditorRunNative::is_deploy_debug_remote_enabled() const {

	return deploy_debug_remote;
}

void EditorRunNative::set_debug_collisions(bool p_debug) {

	debug_collisions = p_debug;
}

bool EditorRunNative::get_debug_collisions() const {

	return debug_collisions;
}

void EditorRunNative::set_debug_navigation(bool p_debug) {

	debug_navigation = p_debug;
}

bool EditorRunNative::get_debug_navigation() const {

	return debug_navigation;
}

EditorRunNative::EditorRunNative() {
	set_process(true);
	first = true;
	deploy_dumb = false;
	deploy_debug_remote = false;
	debug_collisions = false;
	debug_navigation = false;
}
