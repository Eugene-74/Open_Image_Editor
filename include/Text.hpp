#pragma once

#include <QString>
#include <map>

#include "Translation.hpp"
namespace Text {
extern TranslationManager translationManager;

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

inline QString back() {
    return translationManager.translateQ("Text.Tooltip.BACK");
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
};  // namespace ImageEditor
};  // namespace Tooltip
};  // namespace Text
