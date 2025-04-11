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

const std::vector<std::string> filesToDownload = {"coco.names", "yolov5n.onnx", "yolov5s.onnx", "yolov5m.onnx", "yolov5l.onnx", "yolov5x.onnx"};

const int PREVIEW_NBR = 3;
const int PRE_LOAD_RADIUS = 5;  // on en precharge 2 de plus comme ça l'utilisateut ne vois meme pas les images charger
const int TIME_BEFORE_FULL_QUALITY = 200;

const int FREE_THREAD = 2;
// const int TIME_BEFORE_FULL_QUALITY_IMAGE_BOOTH = 500;
const int PRE_LOAD_RADIUS_IMAGE_BOOTH = 5;

const int TIME_UNDO_VISUALISATION = 500;

const int TIME_BEFORE_REZISE = 100;

const std::string CLICK_BACKGROUND_COLOR = "#9c9c9c";
const std::string HOVER_BACKGROUND_COLOR = "#b3b3b3";
const std::string BACKGROUND_COLOR = "transparent";

const QString IMAGE_EDITOR_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Editor Window";
const QString IMAGE_BOOTH_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";
const QString MAIN_WINDOW_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";

const int IMAGE_BOOTH_IMAGE_QUALITY = 256;  // 128 or 256 or 512
const int IMAGE_BOOTH_IMAGE_POOR_QUALITY = 16;

#ifdef _WIN32
const std::string SAVE_PATH = APPDATA_PATH.toStdString() + "/" + APP_NAME;
#else
const std::string SAVE_PATH = HOME_PATH.toStdString() + "/" + APP_NAME;
#endif

const std::string THUMBNAIL_PATH = SAVE_PATH + "/thumbnails";

const std::string IMAGESDATA_SAVE_DATA_PATH = SAVE_PATH + "/save." + APP_NAME;

const std::map<std::string, Option> DEFAULT_OPTIONS = {
    {"Sizes::imageBooth::ImagesPerLine", Option("text", "10")}};
const std::vector<std::string> IMAGE_CONVERTION = {"jpg", "jpeg", "png", "webp", "tiff", "bmp", "heic", "heif"};

const std::vector<int> THUMBNAIL_SIZES = {16, 128, 256, 512};

const std::vector<std::string> IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".svg", ".tiff", ".bmp"};
const std::vector<std::string> VIDEO_EXTENSIONS = {".mp4"};

const std::vector<std::string> TURNABLE_IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".tiff", ".bmp"};
const std::vector<std::string> MIRRORABLE_IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".webp", ".heic", ".heif", ".tiff", ".bmp"};
const std::vector<std::string> METADA_ACTION_IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".heic", ".heif", ".webp", ".tiff"};
const std::vector<std::string> REAL_ACTION_IMAGE_EXTENSIONS = {".png", ".bmp"};

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED = "red";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED = "red";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";

const QString IMAGE_PATH_LOADING = ":/images/loading.png";
const QString IMAGE_PATH_ERROR = ":/images/error.png";

const QString ICON_PATH_DELETE = ":/icons/delete.png";
const QString ICON_PATH_SAVE = ":/icons/save.png";
const QString ICON_PATH_EXPORT = ":/icons/export.png";
const QString ICON_PATH_ROTATE_RIGHT = ":/icons/rotateRight.png";
const QString ICON_PATH_ROTATE_LEFT = ":/icons/rotateLeft.png";
const QString ICON_PATH_MIRROR_UP_DOWN = ":/icons/mirrorUpDown.png";
const QString ICON_PATH_MIRROR_LEFT_RIGHT = ":/icons/mirrorLeftRight.png";
const QString ICON_PATH_CONVERSION = ":/icons/conversion.png";
const QString ICON_PATH_EDIT_EXIF = ":/icons/editExif.png";
const QString ICON_PATH_EDIT_PERSONS = ":/icons/editPersons.png";
const QString ICON_PATH_EDIT_FILTERS = ":/icons/editFilters.png";

const QString ICON_PATH_FOLDER = ":/icons/folder.png";
const QString ICON_PATH_ALL_IMAGES = ":/icons/allImages.png";
const QString ICON_PATH_BACK = ":/icons/back.png";

const QString ICON_PATH_BEFORE = ":/icons/before.png";
const QString ICON_PATH_NEXT = ":/icons/next.png";

const QString ICON_PATH_IMAGE_BOOTH = ":/icons/imageBooth.png";
const QString ICON_PATH_IMAGE_EDITOR = ":/icons/imageEditor.png";
const QString ICON_PATH_OPTION_ADD_IMAGES = ":/icons/addImages.png";

const QString ICON_PATH_DISCORD = ":/icons/discord.png";
const QString ICON_PATH_GITHUB = ":/icons/github.png";
const QString ICON_PATH_OPTION = ":/icons/option.png";

const QString TOOL_TIP_DISCORD = "Open the app discord";
const QString TOOL_TIP_GITHUB = "Open the app github";
const QString TOOL_TIP_PARAMETER = "Open the parameters";

const QString TOOL_TIP_OPEN_IMAGE_BOOTH = "See all the images";
const QString TOOL_TIP_OPEN_IMAGE_EDITOR = "Edit image one by one";
const QString TOOL_TIP_ADD_IMAGES = "Add images to the app";

const QString TOOL_TIP_IMAGE_BOOTH_DELETE = "Delete the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_SAVE = "Save modification";
const QString TOOL_TIP_IMAGE_BOOTH_EXPORT = "Export the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_ROTATE_RIGHT = "Rotate to the right the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_ROTATE_LEFT = "Rotate to the left the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_MIRROR_UP_DOWN = "Mirror UP-DOWN the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the selected images";
const QString TOOL_TIP_IMAGE_BOOTH_CONVERSION = "Convert the selected images";

const QString TOOL_TIP_IMAGE_BOOTH_EDIT_EXIF = "Edit exif data of selected images";
const QString TOOL_TIP_IMAGE_BOOTH_EDIT_PERSONS = "Edit persons data of selected images";
const QString TOOL_TIP_IMAGE_BOOTH_EDIT_FILTERS = "Edit filters";

const QString TOOL_TIP_IMAGE_EDITOR_DELETE = "Delete the image";
const QString TOOL_TIP_IMAGE_EDITOR_SAVE = "Save modification";
const QString TOOL_TIP_IMAGE_EDITOR_EXPORT = "Export the images";
const QString TOOL_TIP_IMAGE_EDITOR_ROTATE_RIGHT = "Rotate to the right the image";
const QString TOOL_TIP_IMAGE_EDITOR_ROTATE_LEFT = "Rotate to the left the image";
const QString TOOL_TIP_IMAGE_EDITOR_MIRROR_UP_DOWN = "Mirror UP-DOWN the image";
const QString TOOL_TIP_IMAGE_EDITOR_MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the image";
const QString TOOL_TIP_IMAGE_EDITOR_EDIT_EXIF = "Edit exif data of the image";
const QString TOOL_TIP_IMAGE_EDITOR_EDIT_PERSONS = "Edit persons data of the image";

const QString TOOL_TIP_IMAGE_BOOTH_ALL_IMAGES = "Open all you images at once";
const QString TOOL_TIP_IMAGE_BOOTH_BACK = "Go to last folder";

// const QString TOOL_TIP_IMAGE_EDITOR_CONVERSION = "Convert the image";
#include <QObject>
const QString TOOL_TIP_IMAGE_EDITOR_CONVERSION = QObject::tr("Convert the image");

class Const {
   public:
    class Orientation {
       public:
        static const int UNDEFINED = 0;
        static const int NORMAL = 1;
        static const int FLIP_HORIZONTAL = 2;
        static const int ROTATE_180 = 3;
        static const int FLIP_VERTICAL = 4;
        static const int TRANSPOSE = 5;
        static const int ROTATE_90 = 6;
        static const int TRANSVERSE = 7;
        static const int ROTATE_270 = 8;
    };
    class Rotation {
       public:
        static const int LEFT = -90;
        static const int RIGHT = 90;
        static const int UP_SIDE_DOWN = 180;
    };
};