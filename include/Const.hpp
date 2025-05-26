#pragma once

#include <QStandardPaths>
#include <QString>
#include <map>
#include <string>

#include "Box.hpp"

const std::string APP_NAME = "OpenImageEditor";

const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float CONFIDENCE_THRESHOLD = 0.4;

const int IMAGE_PER_THREAD = 20;

const QString DESCKTOP_PATH = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
const QString DOCUMENTS_PATH = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
const QString PICTURES_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString HOME_PATH = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
const QString APPDATA_PATH = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
const QString APP_FILES = APPDATA_PATH + "/" + QString::fromStdString(APP_NAME);

const QString DOWNLOAD_PATH = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

const int PREVIEW_NBR = 3;
const int PRE_LOAD_RADIUS = 5;  // on en precharge 2 de plus comme ça l'utilisateut ne vois meme pas les images charger
const int TIME_BEFORE_FULL_QUALITY = 200;

const int IMAGE_BOOTH_PRE_LOAD_RADIUS = 5;  // x pages d'image

const int FREE_THREAD = 4;
const int PRE_LOAD_RADIUS_IMAGE_BOOTH = 5;

const int TIME_UNDO_VISUALISATION = 500;

const int TIME_BEFORE_REZISE = 100;

const std::string THUMBNAIL_PATH = APP_FILES.toStdString() + "/thumbnails";

const std::string IMAGESDATA_SAVE_PATH = APP_FILES.toStdString() + "/save." + APP_NAME;
const std::string OPTIONS_SAVE_PATH = APP_FILES.toStdString() + "/option." + APP_NAME;

const std::map<std::string, Option> DEFAULT_OPTIONS = {
    {"Language", Option("list", "en|en|fr|es")},

    {"Sizes::imageBooth::ImagesPerLine", Option("int", "10")}};

namespace Const {
const int MAX_WORKING_THUMBNAIL = 100;
const int MAX_WORKING_DETECTION = 100;

const int WORKING_THUMBNAIL_TIME = 1000;
const int WORKING_DETECTION_TIME = 10000;

namespace Time {
const int SEC_1 = 1000;
const int MIN_1 = 60 * SEC_1;
};  // namespace Time
namespace Orientation {
static const int UNDEFINED = 0;
static const int NORMAL = 1;
static const int FLIP_HORIZONTAL = 2;
static const int ROTATE_180 = 3;
static const int FLIP_VERTICAL = 4;
static const int TRANSPOSE = 5;
static const int ROTATE_90 = 6;
static const int TRANSVERSE = 7;
static const int ROTATE_270 = 8;
};  // namespace Orientation

namespace Rotation {
static const int LEFT = -90;
static const int RIGHT = 90;
static const int UP_SIDE_DOWN = 180;
};  // namespace Rotation

namespace Thread {
static const int MAX_IN_QUEUE = 100000;
};
namespace Thumbnail {
static constexpr int POOR_QUALITY = 16;
static constexpr int NORMAL_QUALITY = 128;
static constexpr int GOOD_QUALITY = 256;
static constexpr int HIGHT_QUALITY = 512;
static constexpr std::array<int, 4> THUMBNAIL_SIZES = {16, 128, 256, 512};
};  // namespace Thumbnail
namespace Yolo {
static constexpr float INPUT_WIDTH = 640;
static constexpr float INPUT_HEIGHT = 640;
static constexpr float SCORE_THRESHOLD = 0.2;
static constexpr float CONFIDENCE_THRESHOLD = 0.4;
};  // namespace Yolo
namespace ImageBooth {
static const int IMAGE_QUALITY = Const::Thumbnail::GOOD_QUALITY;
static const int IMAGE_POOR_QUALITY = Const::Thumbnail::POOR_QUALITY;
};  // namespace ImageBooth
namespace ImageEditor {

static const int IMAGE_QUALITY = Const::Thumbnail::HIGHT_QUALITY;
static const int IMAGE_POOR_QUALITY = Const::Thumbnail::POOR_QUALITY;
static const int MAX_NAME_LENGTH = 50;
};  // namespace ImageEditor
namespace Extension {

static const std::vector<std::string> IMAGE = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".svg", ".tiff", ".bmp"};
static const std::vector<std::string> VIDEO = {".mp4"};
static const std::vector<std::string> IMAGE_CONVERTION = {"jpg", "jpeg", "png", "webp", "tiff", "bmp", "heic", "heif"};

static const std::vector<std::string> TURNABLE_IMAGE = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".tiff", ".bmp"};
static const std::vector<std::string> MIRRORABLE_IMAGE = {".jpg", ".jpeg", ".png", ".webp", ".heic", ".heif", ".tiff", ".bmp"};
static const std::vector<std::string> METADA_ACTION_IMAGE = {".jpg", ".jpeg", ".heic", ".heif", ".webp", ".tiff"};
static const std::vector<std::string> REAL_ACTION_IMAGE = {".png", ".bmp"};
};  // namespace Extension

namespace ImagePath {
static const QString LOADING = ":/images/loading.png";
static const QString ERROR_PATH = ":/images/error.png";  // can't use ERROR because windows use it
};  // namespace ImagePath
namespace IconPath {
static const QString DELETE_ICON = ":/icons/delete.png";  // can't use DELETE because windows use it
static const QString SAVE = ":/icons/save.png";
static const QString EXPORT = ":/icons/export.png";
static const QString ROTATE_RIGHT = ":/icons/rotateRight.png";
static const QString ROTATE_LEFT = ":/icons/rotateLeft.png";
static const QString MIRROR_UP_DOWN = ":/icons/mirrorUpDown.png";
static const QString MIRROR_LEFT_RIGHT = ":/icons/mirrorLeftRight.png";
static const QString CONVERSION = ":/icons/conversion.png";
static const QString GIMP = ":/icons/gimp.png";
static const QString MAP = ":/icons/map.png";
static const QString EDIT_PERSONS = ":/icons/editPersons.png";
static const QString EDIT_FILTERS = ":/icons/editFilters.png";

static const QString FOLDER = ":/icons/folder.png";
static const QString ALL_IMAGES = ":/icons/allImages.png";
static const QString BACK = ":/icons/back.png";

static const QString BEFORE = ":/icons/before.png";
static const QString NEXT = ":/icons/next.png";

static const QString IMAGE_BOOTH = ":/icons/imageBooth.png";
static const QString ADD_IMAGES = ":/icons/addImages.png";

static const QString HOME = ":/icons/home.png";

static const QString WIFI = ":/icons/wifi.png";
static const QString NO_WIFI = ":/icons/noWifi.png";
static const QString DISCORD = ":/icons/discord.png";
static const QString GITHUB = ":/icons/github.png";
static const QString OPTION = ":/icons/option.png";
namespace Language {
static const QString EN = ":/icons/en.png";
static const QString FR = ":/icons/fr.png";
static const QString ES = ":/icons/es.png";

}  // namespace Language
};  // namespace IconPath

namespace Color {
static const std::string TRANSPARENT1 = "transparent";
static const std::string LIGHT_GRAY = "#b3b3b3";
static const std::string GRAY = "#9c9c9c";
static const std::string WHITE = "#FFFFFF";
static const std::string RED = "#FF0000";
static const std::string GREEN = "#00FF00";
static const std::string BLUE = "#FF0000";
static const std::string DARK = "#000000";

static const std::string DARK_RED = "#700c13";
static const std::string LIGHT_RED = "#F00c13";

};  // namespace Color
namespace Model {
namespace YoloV5 {

static const std::string GITHUB_TAG = "yolov5";

namespace Names {
static const std::string CLASS = "class.names";

static const std::string N = "yolov5n.onnx";
static const std::string S = "yolov5s.onnx";
static const std::string M = "yolov5m.onnx";
static const std::string L = "yolov5l.onnx";
static const std::string X = "yolov5x.onnx";
};  // namespace Names
const std::map<std::string, int> QUALITY = {
    {Const::Model::YoloV5::Names::N, 1},  // nano
    {Const::Model::YoloV5::Names::S, 2},  // small
    {Const::Model::YoloV5::Names::M, 3},  // medium
    {Const::Model::YoloV5::Names::L, 4},  // large
    {Const::Model::YoloV5::Names::X, 5}   // xlarge
};
};  // namespace YoloV5
namespace Arcface {
static const std::string GITHUB_TAG = "arcface";

static const std::string NAME = "arcface.onnx";

};  // namespace Arcface

namespace Haarcascade {
static const std::string GITHUB_TAG = "haarcascade";

static const std::string NAME = "haarcascade_frontalface_alt2.xml";

};  // namespace Haarcascade
};  // namespace Model

};  // namespace Const

const std::string CLICK_BACKGROUND_COLOR = Const::Color::GRAY;
const std::string HOVER_BACKGROUND_COLOR = Const::Color::LIGHT_GRAY;
const std::string BACKGROUND_COLOR = Const::Color::TRANSPARENT1;

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED = Const::Color::RED;
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED = Const::Color::RED;

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = Const::Color::BLUE;
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = Const::Color::BLUE;

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = Const::Color::GREEN;
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = Const::Color::GREEN;

