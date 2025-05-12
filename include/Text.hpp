#pragma once

#include <QString>
#include <map>

#include "Translation.hpp"
namespace Text {
extern TranslationManager translationManager;
static const QString VALIDATE = translationManager.translateQ("Text.VALIDATE");
namespace Tooltip {
static const QString DISCORD = "Open the app discord";
static const QString GITHUB = "Open the app github";
static const QString PARAMETER = "Open the parameters";

static const QString OPEN_IMAGE_BOOTH = "See all the images";
static const QString OPEN_IMAGE_EDITOR = "Edit image one by one";
static const QString ADD_IMAGES = "Add images to the app";

static const QString BACK = "Go to last folder";

namespace ImageBooth {
static const QString DELETE_TIP = "Delete the selected images";  // can't use DELETE because windows use it
static const QString SAVE = "Save modification";
static const QString EXPORT = "Export the selected images";
static const QString ROTATE_RIGHT = "Rotate to the right the selected images";
static const QString ROTATE_LEFT = "Rotate to the left the selected images";
static const QString MIRROR_UP_DOWN = "Mirror UP-DOWN the selected images";
static const QString MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the selected images";
static const QString CONVERSION = "Convert the selected images";

static const QString MAP = "Edit map data of selected images";
static const QString EDIT_PERSONS = "Edit persons data of selected images";
static const QString EDIT_FILTERS = "Edit filters";

static const QString ALL_IMAGES = "Open all you images at once";
};  // namespace ImageBooth
namespace ImageEditor {
static const QString DELETE_TIP = "Delete the image";  // can't use DELETE because windows use it
static const QString SAVE = "Save modification";
static const QString EXPORT = "Export the images";
static const QString ROTATE_RIGHT = "Rotate to the right the image";
static const QString ROTATE_LEFT = "Rotate to the left the image";
static const QString MIRROR_UP_DOWN = "Mirror UP-DOWN the image";
static const QString MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the image";
static const QString MAP = "Edit map data of the image";
static const QString EDIT_PERSONS = "Edit persons data of the image";
static const QString CONVERSION = "Convert the image";
};  // namespace ImageEditor
};  // namespace Tooltip
};  // namespace Text
