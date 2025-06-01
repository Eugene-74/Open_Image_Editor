#pragma once

#include <QString>
#include <map>

#include "Translation.hpp"
namespace Text {
extern TranslationManager translationManager;

inline QString welcome() {
    return translationManager.translateQ("Text.WELCOME");
}

inline QString validate() {
    return translationManager.translateQ("Text.VALIDATE");
}

inline QString name() {
    return translationManager.translateQ("Text.NAME");
}

inline QString date() {
    return translationManager.translateQ("Text.DATE");
}

inline QString geolocation() {
    return translationManager.translateQ("Text.GEOLOCATION");
}
namespace Tooltip {
inline QString language() {
    return translationManager.translateQ("Text.Tooltip.LANGUAGE");
}

inline QString wifi() {
    return translationManager.translateQ("Text.Tooltip.WIFI");
}
inline QString noWifi() {
    return translationManager.translateQ("Text.Tooltip.NO_WIFI");
}
inline QString discord() {
    return translationManager.translateQ("Text.Tooltip.DISCORD");
}
inline QString github() {
    return translationManager.translateQ("Text.Tooltip.GITHUB");
}
inline QString option() {
    return translationManager.translateQ("Text.Tooltip.PARAMETER");
}

inline QString open_image_booth() {
    return translationManager.translateQ("Text.Tooltip.OPEN_IMAGE_BOOTH");
}
inline QString open_image_editor() {
    return translationManager.translateQ("Text.Tooltip.OPEN_IMAGE_EDITOR");
}
inline QString add_images() {
    return translationManager.translateQ("Text.Tooltip.ADD_IMAGES");
}

inline QString back_folder() {
    return translationManager.translateQ("Text.Tooltip.BACK_FOLDER");
}

inline QString back_window() {
    return translationManager.translateQ("Text.Tooltip.BACK_FOLDER");
}

inline QString home() {
    return translationManager.translateQ("Text.Tooltip.HOME");
}

namespace ImageBooth {
inline QString delete_tip() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.DELETE_TIP");
}
inline QString save() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.SAVE");
}
inline QString export_tip() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.EXPORT");
}
inline QString rotate_right() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.ROTATE_RIGHT");
}
inline QString rotate_left() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.ROTATE_LEFT");
}
inline QString mirror_up_down() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.MIRROR_UP_DOWN");
}
inline QString mirror_left_right() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.MIRROR_LEFT_RIGHT");
}
inline QString conversion() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.CONVERSION");
}

inline QString map() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.MAP");
}
inline QString edit_persons() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.EDIT_PERSONS");
}
inline QString edit_filters() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.EDIT_FILTERS");
}

inline QString all_images() {
    return translationManager.translateQ("Text.Tooltip.ImageBooth.ALL_IMAGES");
}
};  // namespace ImageBooth

namespace ImageEditor {
inline QString delete_tip() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.DELETE_TIP");
}
inline QString save() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.SAVE");
}
inline QString export_tip() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.EXPORT");
}
inline QString rotate_right() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.ROTATE_RIGHT");
}
inline QString rotate_left() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.ROTATE_LEFT");
}
inline QString mirror_up_down() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.MIRROR_UP_DOWN");
}
inline QString mirror_left_right() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.MIRROR_LEFT_RIGHT");
}
inline QString map() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.MAP");
}
inline QString edit_persons() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.EDIT_PERSONS");
}
inline QString conversion() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.CONVERSION");
}
inline QString gimp() {
    return translationManager.translateQ("Text.Tooltip.ImageEditor.GIMP");
}

};  // namespace ImageEditor
};  // namespace Tooltip
namespace Error {
inline QString failedLoadModel() {
    return translationManager.translateQ("Text.Error.FAILED_LOAD_MODEL");
}
inline QString failedDownloadModel() {
    return translationManager.translateQ("Text.Error.FAILED_DOWNLOAD_MODEL");
}
};  // namespace Error
namespace Option {
namespace ImageEditor {
inline QString editionFaceWindow() {
    return translationManager.translateQ("Text.Option.ImageEditor.EDITION_FACE_WINDOW");
}
inline QString renameFace() {
    return translationManager.translateQ("Text.Option.ImageEditor.RENAME_FACE");
}
inline QString deleteFace() {
    return translationManager.translateQ("Text.Option.ImageEditor.DELETE_FACE");
}
inline QString mergeFace() {
    return translationManager.translateQ("Text.Option.ImageEditor.MERGE_FACE");
}
};  // namespace ImageEditor
namespace Loading {
inline QString keepOldSave() {
    return translationManager.translateQ("Text.Option.Loading.KEEP_OLD_SAVE");
}
}  // namespace Loading
}  // namespace Option
};  // namespace Text
