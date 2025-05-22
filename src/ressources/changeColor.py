from PIL import Image
import os

def change_image_color(image_path, output_path, new_color):
    """
    Change all non-transparent pixels of the image to the specified new color.

    :param image_path: Path to the input image file.
    :param output_path: Path to save the modified image.
    :param new_color: Tuple representing the new color (R, G, B, A).
    """
    with Image.open(image_path) as img:
        img = img.convert("RGBA")
        data = img.getdata()

        new_data = []
        for item in data:
            if item[3] != 0:
                new_data.append(new_color)
            else:
                new_data.append(item)

        img.putdata(new_data)
        img.save(output_path)

def change_image_color_of_folder(input_directory,output_directory,r,g,b,a):
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # Iterate over all files in the input directory
    for filename in os.listdir(input_directory):
        if filename.endswith('.png') or filename.endswith('.ico'):
            input_path = os.path.join(input_directory, filename)
            output_path = os.path.join(output_directory, filename)
            change_image_color(input_path, output_path, (r, g, b, a))  # Change to red color


def createIconsAndImages(r,g,b,a):
    input_directory = "src/ressources/icons"
    output_directory = "src/ressources/"+str(r)+"-"+str(g)+"-"+str(b)+"-"+str(a)+"/icons"
    change_image_color_of_folder(input_directory,output_directory,r,g,b,a)

    input_directory = "src/ressources/images"
    output_directory = "src/ressources/"+str(r)+"-"+str(g)+"-"+str(b)+"-"+str(a)+"/images"
    change_image_color_of_folder(input_directory,output_directory,r,g,b,a)

r,g,b,a=0,0,0,255
createIconsAndImages(r,g,b,a)

r,g,b,a=255,255,255,255
createIconsAndImages(r,g,b,a)

print("Icons and images have been created in the folders src/ressources/0-0-0-255/icons and src/ressources/255-255-255-255/icons")