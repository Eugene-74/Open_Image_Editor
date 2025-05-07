#include "Const.hpp"

#include <string>
#include <vector>

// --- EXTENSION ---
const std::vector<std::string> Const::Extension::IMAGE = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".svg", ".tiff", ".bmp"};
const std::vector<std::string> Const::Extension::VIDEO = {".mp4"};
const std::vector<std::string> Const::Extension::IMAGE_CONVERTION = {"jpg", "jpeg", "png", "webp", "tiff", "bmp", "heic", "heif"};
const std::vector<std::string> Const::Extension::TURNABLE_IMAGE = {".jpg", ".jpeg", ".png", ".heic", ".heif", ".webp", ".tiff", ".bmp"};
const std::vector<std::string> Const::Extension::MIRRORABLE_IMAGE = {".jpg", ".jpeg", ".png", ".webp", ".heic", ".heif", ".tiff", ".bmp"};
const std::vector<std::string> Const::Extension::METADA_ACTION_IMAGE = {".jpg", ".jpeg", ".heic", ".heif", ".webp", ".tiff"};
const std::vector<std::string> Const::Extension::REAL_ACTION_IMAGE = {".png", ".bmp"};

// --- IMAGEPATH ---
const QString Const::ImagePath::LOADING = ":/images/loading.png";
const QString Const::ImagePath::ERROR_PATH = ":/images/error.png";

// --- ICONPATH ---
const QString Const::IconPath::DELETE_ICON = ":/icons/delete.png";
const QString Const::IconPath::SAVE = ":/icons/save.png";
const QString Const::IconPath::EXPORT = ":/icons/export.png";
const QString Const::IconPath::ROTATE_RIGHT = ":/icons/rotateRight.png";
const QString Const::IconPath::ROTATE_LEFT = ":/icons/rotateLeft.png";
const QString Const::IconPath::MIRROR_UP_DOWN = ":/icons/mirrorUpDown.png";
const QString Const::IconPath::MIRROR_LEFT_RIGHT = ":/icons/mirrorLeftRight.png";
const QString Const::IconPath::CONVERSION = ":/icons/conversion.png";
const QString Const::IconPath::MAP = ":/icons/map.png";
const QString Const::IconPath::EDIT_PERSONS = ":/icons/editPersons.png";
const QString Const::IconPath::EDIT_FILTERS = ":/icons/editFilters.png";

const QString Const::IconPath::FOLDER = ":/icons/folder.png";
const QString Const::IconPath::ALL_IMAGES = ":/icons/allImages.png";
const QString Const::IconPath::BACK = ":/icons/back.png";

const QString Const::IconPath::BEFORE = ":/icons/before.png";
const QString Const::IconPath::NEXT = ":/icons/next.png";

const QString Const::IconPath::IMAGE_BOOTH = ":/icons/imageBooth.png";
const QString Const::IconPath::ADD_IMAGES = ":/icons/addImages.png";

const QString Const::IconPath::DISCORD = ":/icons/discord.png";
const QString Const::IconPath::GITHUB = ":/icons/github.png";
const QString Const::IconPath::OPTION = ":/icons/option.png";

// --- TOOPTIP ---
const QString Const::Tooltip::DISCORD = "Open the app discord";
const QString Const::Tooltip::GITHUB = "Open the app github";
const QString Const::Tooltip::PARAMETER = "Open the parameters";

const QString Const::Tooltip::OPEN_IMAGE_BOOTH = "See all the images";
const QString Const::Tooltip::OPEN_IMAGE_EDITOR = "Edit image one by one";
const QString Const::Tooltip::ADD_IMAGES = "Add images to the app";
const QString Const::Tooltip::BACK = "Go to last folder";

const QString Const::Tooltip::ImageBooth::DELETE_TIP = "Delete the selected images";
const QString Const::Tooltip::ImageBooth::SAVE = "Save modification";
const QString Const::Tooltip::ImageBooth::EXPORT = "Export the selected images";
const QString Const::Tooltip::ImageBooth::ROTATE_RIGHT = "Rotate to the right the selected images";
const QString Const::Tooltip::ImageBooth::ROTATE_LEFT = "Rotate to the left the selected images";
const QString Const::Tooltip::ImageBooth::MIRROR_UP_DOWN = "Mirror UP-DOWN the selected images";
const QString Const::Tooltip::ImageBooth::MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the selected images";
const QString Const::Tooltip::ImageBooth::CONVERSION = "Convert the selected images";
const QString Const::Tooltip::ImageBooth::MAP = "Edit map data of selected images";
const QString Const::Tooltip::ImageBooth::EDIT_PERSONS = "Edit persons data of selected images";
const QString Const::Tooltip::ImageBooth::EDIT_FILTERS = "Edit filters";
const QString Const::Tooltip::ImageBooth::ALL_IMAGES = "Open all you images at once";

const QString Const::Tooltip::ImageEditor::DELETE_TIP = "Delete the image";
const QString Const::Tooltip::ImageEditor::SAVE = "Save modification";
const QString Const::Tooltip::ImageEditor::EXPORT = "Export the images";
const QString Const::Tooltip::ImageEditor::ROTATE_RIGHT = "Rotate to the right the image";
const QString Const::Tooltip::ImageEditor::ROTATE_LEFT = "Rotate to the left the image";
const QString Const::Tooltip::ImageEditor::MIRROR_UP_DOWN = "Mirror UP-DOWN the image";
const QString Const::Tooltip::ImageEditor::MIRROR_LEFT_RIGHT = "Mirror LEFT-RIGHT the image";
const QString Const::Tooltip::ImageEditor::MAP = "Edit map data of the image";
const QString Const::Tooltip::ImageEditor::EDIT_PERSONS = "Edit persons data of the image";
const QString Const::Tooltip::ImageEditor::CONVERSION = "Convert the image";
