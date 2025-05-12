#pragma once

#include <QString>
#include <map>

#include "Translation.hpp"
namespace Text {
extern TranslationManager translationManager;
static const QString VALIDATE = translationManager.translateQ("Text.VALIDATE");
namespace Tooltip {
static const QString DISCORD = translationManager.translateQ("Text.Tooltip.DISCORD");
static const QString GITHUB = translationManager.translateQ("Text.Tooltip.GITHUB");
static const QString PARAMETER = translationManager.translateQ("Text.Tooltip.PARAMETER");

static const QString OPEN_IMAGE_BOOTH = translationManager.translateQ("Text.Tooltip.OPEN_IMAGE_BOOTH");
static const QString OPEN_IMAGE_EDITOR = translationManager.translateQ("Text.Tooltip.OPEN_IMAGE_EDITOR");
static const QString ADD_IMAGES = translationManager.translateQ("Text.Tooltip.ADD_IMAGES");

static const QString BACK = translationManager.translateQ("Text.Tooltip.BACK");

namespace ImageBooth {
static const QString DELETE_TIP = translationManager.translateQ("Text.Tooltip.ImageBooth.DELETE_TIP");
static const QString SAVE = translationManager.translateQ("Text.Tooltip.ImageBooth.SAVE");
static const QString EXPORT = translationManager.translateQ("Text.Tooltip.ImageBooth.EXPORT");
static const QString ROTATE_RIGHT = translationManager.translateQ("Text.Tooltip.ImageBooth.ROTATE_RIGHT");
static const QString ROTATE_LEFT = translationManager.translateQ("Text.Tooltip.ImageBooth.ROTATE_LEFT");
static const QString MIRROR_UP_DOWN = translationManager.translateQ("Text.Tooltip.ImageBooth.MIRROR_UP_DOWN");
static const QString MIRROR_LEFT_RIGHT = translationManager.translateQ("Text.Tooltip.ImageBooth.MIRROR_LEFT_RIGHT");
static const QString CONVERSION = translationManager.translateQ("Text.Tooltip.ImageBooth.CONVERSION");

static const QString MAP = translationManager.translateQ("Text.Tooltip.ImageBooth.MAP");
static const QString EDIT_PERSONS = translationManager.translateQ("Text.Tooltip.ImageBooth.EDIT_PERSONS");
static const QString EDIT_FILTERS = translationManager.translateQ("Text.Tooltip.ImageBooth.EDIT_FILTERS");

static const QString ALL_IMAGES = translationManager.translateQ("Text.Tooltip.ImageBooth.ALL_IMAGES");
};  // namespace ImageBooth
namespace ImageEditor {
static const QString DELETE_TIP = translationManager.translateQ("Text.Tooltip.ImageEditor.DELETE_TIP");
static const QString SAVE = translationManager.translateQ("Text.Tooltip.ImageEditor.SAVE");
static const QString EXPORT = translationManager.translateQ("Text.Tooltip.ImageEditor.EXPORT");
static const QString ROTATE_RIGHT = translationManager.translateQ("Text.Tooltip.ImageEditor.ROTATE_RIGHT");
static const QString ROTATE_LEFT = translationManager.translateQ("Text.Tooltip.ImageEditor.ROTATE_LEFT");
static const QString MIRROR_UP_DOWN = translationManager.translateQ("Text.Tooltip.ImageEditor.MIRROR_UP_DOWN");
static const QString MIRROR_LEFT_RIGHT = translationManager.translateQ("Text.Tooltip.ImageEditor.MIRROR_LEFT_RIGHT");
static const QString MAP = translationManager.translateQ("Text.Tooltip.ImageEditor.MAP");
static const QString EDIT_PERSONS = translationManager.translateQ("Text.Tooltip.ImageEditor.EDIT_PERSONS");
static const QString CONVERSION = translationManager.translateQ("Text.Tooltip.ImageEditor.CONVERSION");
};  // namespace ImageEditor
};  // namespace Tooltip
};  // namespace Text
