#!/usr/bin/env python3
import os
import argparse
from PIL import Image

def generate_xbm_from_folder(folder_path, output_name, output_folder):
    # Ensure the output folder exists
    os.makedirs(output_folder, exist_ok=True)

    # List all files in the folder
    image_files = [f for f in os.listdir(folder_path) if os.path.isfile(os.path.join(folder_path, f))]

    # Initialize the content for the header and C source files
    header_content = f"#ifndef {output_name.upper()}_H\n#define {output_name.upper()}_H\n\n"
    c_content = f'#include "{output_name}.h"\n\n'
    
    for image_file in image_files:
        # Get the full path of the image
        image_path = os.path.join(folder_path, image_file)
        
        # Open the image and convert it to 1-bit (black and white)
        img = Image.open(image_path).convert('1')
        
        # Ensure the image dimensions are multiples of 8 (for XBM formatting)
        width, height = img.size
        if width % 8 != 0 or height % 8 != 0:
            new_width = (width + 7) // 8 * 8  # Round up to the nearest multiple of 8
            new_height = (height + 7) // 8 * 8
            img = img.resize((new_width, new_height))
        
        # Get the base name of the file (without extension)
        base_name = os.path.splitext(image_file)[0]
        
        # Save image as XBM to a temporary file and read the contents

        # Generate a C array from the XBM data
        xbm_data = img.convert("1").tobitmap(f"{base_name}_xbm_image")
        # Get the everything in str(xbm_data) after the first '{' and before the last '}'
        xbm_bits = str(xbm_data).split("{")[1].split("}")[0]
        # Translate \n to enter
        xbm_bits = xbm_bits.replace("\\n", "\n")
                
        # Add the XBM data to the header and C source files
        header_content += f"extern const unsigned char {base_name.upper()}_XBM_IMAGE[];\n"
        header_content += f"extern const unsigned int {base_name.upper()}_XBM_WIDTH;\n"
        header_content += f"extern const unsigned int {base_name.upper()}_XBM_HEIGHT;\n\n"

        c_content += f"const unsigned char {base_name.upper()}_XBM_IMAGE[] = {{\n"
        c_content += xbm_bits + "\n};\n"
        c_content += f"const unsigned int {base_name.upper()}_XBM_WIDTH = {width};\n"
        c_content += f"const unsigned int {base_name.upper()}_XBM_HEIGHT = {height};\n\n"

    # Finalize the header content
    header_content += f"#endif  // {output_name.upper()}_H\n"

    # Write the header and C files to the output folder
    header_file_path = os.path.join(output_folder, f"{output_name}.h")
    c_file_path = os.path.join(output_folder, f"{output_name}.c")

    with open(header_file_path, "w") as h_file:
        h_file.write(header_content.strip())
    
    with open(c_file_path, "w") as c_file:
        c_file.write(c_content.strip())
    
    print(f"Generated {output_name}.h and {output_name}.c in folder '{output_folder}' successfully.")

if __name__ == "__main__":
    # Create argument parser
    parser = argparse.ArgumentParser(description="Generate XBM files from images in a folder")
    
    # Add folder argument
    parser.add_argument("folder", help="Path to the folder containing images")
    
    # Add output name argument
    parser.add_argument("output_name", help="Base name for the output files")
    
    # Add output folder argument
    parser.add_argument("output_folder", help="Path to the folder to save the output files")
    
    # Parse the arguments
    args = parser.parse_args()
    
    # Generate XBM files from images in the folder
    generate_xbm_from_folder(args.folder, args.output_name, args.output_folder)
