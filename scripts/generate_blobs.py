import os
import argparse
from flash_algo import PackFlashAlgo

# TODO
# FIXED LENGTH - remove and these (shrink offset to 4 for bkpt only)
BLOB_HEADER = '0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,'
HEADER_SIZE = 0x20

def str_to_num(val):
    return int(val,0)  #convert string to number and automatically handle hex conversion

def main():
    parser = argparse.ArgumentParser(description="Blob generator")
    parser.add_argument("elf_path", help="Elf, axf, or flm to extract "
                        "flash algo from")
    parser.add_argument("--blob_start", default=0x20000000, type=str_to_num, help="Starting "
                        "address of the flash blob. Used only for DAPLink.")
    args = parser.parse_args()

    with open(args.elf_path, "rb") as file_handle:
        algo = PackFlashAlgo(file_handle.read())

    print(algo.flash_info)

    template_dir = os.path.dirname(os.path.realpath(__file__))
    output_dir = os.path.dirname(args.elf_path)
    SP = args.blob_start + 2048
    data_dict = {
        'name': os.path.splitext(os.path.split(args.elf_path)[-1])[0],
        'prog_header': BLOB_HEADER,
        'header_size': HEADER_SIZE,
        'entry': args.blob_start,
        'stack_pointer': SP,
    }

    tmpl_name_list = [
        ("c_blob.tmpl", "c_blob.c"),
        ("py_blob.tmpl", "py_blob.py"),
        ("c_blob_mbed.tmpl", "c_blob_mbed.c")
    ]

    for tmpl, name in tmpl_name_list:
        template_path = os.path.join(template_dir, tmpl)
        output_path = os.path.join(output_dir, name)
        algo.process_template(template_path, output_path, data_dict)


if __name__ == '__main__':
    main()
