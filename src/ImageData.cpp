#include "ImageData.hpp"

ImageData& ImageData::operator=(const ImageData& other) {
    if (this != &other) {
        folders.getName() = other.folders.getName();
        metaData = other.metaData;
        cropSizes = other.cropSizes;
        orientation = other.orientation;
        persons = other.persons;
        personStatus = other.personStatus;
        date = other.date;
    }
    return *this;
}

bool ImageData::operator==(const ImageData& other) const {
    return this->folders == other.folders &&
           this->metaData == other.metaData &&
           this->cropSizes == other.cropSizes &&
           this->orientation == other.orientation &&
           this->persons == other.persons &&
           this->personStatus == other.personStatus &&
           this->date == other.date;
}

void ImageData::print() const {
    qDebug() << get();
}

std::string ImageData::get() const {
    std::string name;
    name += "Image : " + getImagePathConst() + " folders : ";
    for (const auto& file : folders.getFoldersConst()) {
        name += " ";
        name += file.getName();
    }
    name += "\n";
    return name;
}

MetaData* ImageData::getMetaDataPtr() {
    return &metaData;
}

MetaData ImageData::getMetaData() const {
    return metaData;
}

std::vector<Folders> ImageData::getFolders() {
    return *folders.getFolders();
}


void ImageData::addFolder(const std::string& toAddFolder) {
    folders.addFolder(toAddFolder);
}

void ImageData::addFolders(const std::vector<std::string>& toAddFolders) {
    for (const auto& folder : toAddFolders) {
        folders.addFolder(folder);
    }
}

std::string ImageData::getImageName() const {
    std::filesystem::path filePath(getImagePathConst());
    return filePath.filename().string();
}

std::string ImageData::getImagePath() {
    return folders.getName();
}

std::string ImageData::getImagePathConst() const {
    return folders.getName();
}

std::string ImageData::getImageExtension() {
    return fs::path(getImagePath()).extension().string();
}

void ImageData::setExifMetaData(const Exiv2::ExifData& toAddMetaData) {
    try {
        metaData.setExifData(toAddMetaData);
        saveMetaData();
    } catch (const Exiv2::Error& e) {
        qDebug() << "Exiv2 error: " << e.what();
    }
}

void ImageData::loadData() {
    try {
        if (!metaData.dataLoaded) {
            metaData.loadData(getImagePath());
            orientation = metaData.getImageOrientation();
            date = metaData.getTimestamp();

            metaData.dataLoaded = true;
        }
    } catch (const Exiv2::Error& e) {
        qDebug() << "Error loading metadata for image: " << getImagePath();
        qDebug() << "Exiv2 error: " << e.what();
    }
}

void ImageData::saveMetaData() {
    try {
        metaData.saveMetaData(getImagePath());
    } catch (const Exiv2::Error& e) {
        qDebug() << "Exiv2 error: " << e.what();
    }
}

int ImageData::getImageWidth() {
    return metaData.getImageWidth();
}
int ImageData::getImageHeight() {
    return metaData.getImageHeight();
}
int ImageData::getImageOrientation() {
    return metaData.getImageOrientation();
}

void ImageData::turnImage(int rotation) {
    // qDebug() << "Turn image from  : " << orientation << " to " << rotation;
    this->orientation = rotation;
    metaData.modifyExifValue("Exif.Image.Orientation", std::to_string(rotation));
    // metaData.modifyExifValue("Exif.Thumbnail.Orientation", std::to_string(rotation));
    // metaData.modifyXmpValue("Xmp.tiff.Orientation", std::to_string(rotation));
}

void ImageData::setOrCreateExifData() {
    metaData.setOrCreateExifData(getImagePath());
}

void ImageData::save(std::ofstream& out) const {
    out.write(reinterpret_cast<const char*>(&orientation), sizeof(orientation));
    out.write(reinterpret_cast<const char*>(&date), sizeof(date));

    folders.save(out);

    size_t cropSizesSize = cropSizes.size();
    out.write(reinterpret_cast<const char*>(&cropSizesSize), sizeof(cropSizesSize));
    for (const auto& cropSize : cropSizes) {
        size_t innerSize = cropSize.size();
        out.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        out.write(reinterpret_cast<const char*>(cropSize.data()), innerSize * sizeof(QPoint));
    }

    out.write(reinterpret_cast<const char*>(&personStatus), sizeof(personStatus));

    size_t personsSize = persons.size();
    out.write(reinterpret_cast<const char*>(&personsSize), sizeof(personsSize));
    for (const auto& person : persons) {
        qDebug() << "save person : " << folders.getName();
        person.save(out);
    }
}

void ImageData::load(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(&orientation), sizeof(orientation));
    in.read(reinterpret_cast<char*>(&date), sizeof(date));

    folders.load(in);

    size_t cropSizesSize;
    in.read(reinterpret_cast<char*>(&cropSizesSize), sizeof(cropSizesSize));
    if (cropSizesSize >= 1) {
        cropSizes.resize(cropSizesSize);
        for (size_t i = 0; i < cropSizesSize; ++i) {
            size_t innerSize;
            in.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
            cropSizes[i].resize(innerSize);
            in.read(reinterpret_cast<char*>(cropSizes[i].data()), innerSize * sizeof(QPoint));
        }
    }
    in.read(reinterpret_cast<char*>(&personStatus), sizeof(personStatus));
    // TODO mettre autre part
    if (isPersonStatusLoading()) {
        setPersonStatusNotLoaded();
    }

    size_t personsSize;
    in.read(reinterpret_cast<char*>(&personsSize), sizeof(personsSize));
    persons.resize(personsSize);
    for (auto& person : persons) {
        qDebug() << "load person" << folders.getName();
        person.load(in);
    }
    if (persons.size() > 0) {
        // setPersonStatusLoaded();
        qDebug() << "load person done" << persons.size();
    }
}

std::vector<std::vector<QPoint>> ImageData::getCropSizes() const {
    return cropSizes;
}

void ImageData::setCropSizes(const std::vector<std::vector<QPoint>>& cropSizes) {
    this->cropSizes = cropSizes;
}

void ImageData::clearMetaData() {
    metaData.clear();
    metaData.dataLoaded = false;
}

ImageData::PersonStatus ImageData::getPersonStatus() const {
    return personStatus;
}

void ImageData::setPersonStatus(PersonStatus personStatus) {
    this->personStatus = personStatus;
}

void ImageData::setPersonStatusLoading() {
    setPersonStatus(PersonStatus::Loading);
}

void ImageData::setPersonStatusNotLoaded() {
    setPersonStatus(PersonStatus::NotLoaded);
}

void ImageData::setPersonStatusLoaded() {
    setPersonStatus(PersonStatus::Loaded);
}

bool ImageData::isPersonStatusLoading() {
    return personStatus == PersonStatus::Loading;
}

bool ImageData::isPersonStatusNotLoaded() {
    return personStatus == ImageData::PersonStatus::NotLoaded;
}

bool ImageData::isPersonStatusLoaded() {
    return personStatus == ImageData::PersonStatus::Loaded;
}

void ImageData::setMetaData(const MetaData& metaData) {
    this->metaData = metaData;
}

std::vector<Person> ImageData::getpersons() const {
    return persons;
}

void ImageData::setpersons(const std::vector<Person>& persons) {
    this->persons = persons;
}

void ImageData::setDate(long date) {
    this->date = date;
}

long ImageData::getDate() const {
    return date;
}

void ImageData::setOrientation(int orientation) {
    this->orientation = orientation;
}

int ImageData::getOrientation() const {
    return orientation;
}