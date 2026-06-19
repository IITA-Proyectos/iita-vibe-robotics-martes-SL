#!/usr/bin/env python3
"""
Utility script to rename slide image files to the project standard format:
claseX_diapositivaYY.png

Example:
  "Clase 9 - Parámetros-01.png" -> "clase9_diapositiva01.png"
"""

import os
import sys

def rename_slides(img_dir, class_number, prefix_filter):
    if not os.path.isdir(img_dir):
        print(f"Error: Directory '{img_dir}' does not exist.")
        return
        
    print(f"Scanning directory: {img_dir} for files starting with '{prefix_filter}'")
    files = os.listdir(img_dir)
    count = 0
    for f in files:
        if f.startswith(prefix_filter) and f.lower().endswith(".png"):
            # Extract number from filename (e.g., last part after hyphen)
            parts = f.split("-")
            if len(parts) >= 2:
                num_str = parts[-1].replace(".png", "").strip()
                # Pads the number to 2 digits if it's not already
                if len(num_str) == 1 and num_str.isdigit():
                    num_str = f"0{num_str}"
                
                new_name = f"clase{class_number}_diapositiva{num_str}.png"
                src = os.path.join(img_dir, f)
                dst = os.path.join(img_dir, new_name)
                try:
                    os.rename(src, dst)
                    print(f"Renamed: {f} -> {new_name}")
                    count += 1
                except Exception as e:
                    print(f"Failed to rename {f}: {e}")
                    
    print(f"Successfully renamed {count} files.")

if __name__ == '__main__':
    # Default values for Clase 9 slides
    default_img_dir = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "slides_python", "img")
    )
    
    img_dir = sys.argv[1] if len(sys.argv) > 1 else default_img_dir
    class_num = sys.argv[2] if len(sys.argv) > 2 else "9"
    prefix = sys.argv[3] if len(sys.argv) > 3 else "Clase 9 -"
    
    rename_slides(img_dir, class_num, prefix)
