#!/bin/python
from p4utils.utils.sswitch_thrift_API import SimpleSwitchThriftAPI
import argparse
import json
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
        return data

def clear_tables_and_registers(p4api):
    # clear table
    tables = p4api.get_tables()
    for table in tables:
        print(f"Clearing table: {table}")
        p4api.table_clear(table)

    # clear regs
    json_file_path = "../leafswitch/netbufferv4.json"  # 替换为实际的 JSON 文件路径
    try:
        json_data = read_json_file(json_file_path)
        if "register_arrays" in json_data:
            register_arrays = json_data["register_arrays"]
            for register_array in register_arrays:
                name = register_array.get("name")
                p4api.register_reset(name)
                print(f"Clearing register: {name}")
        else:
            print("No 'register_arrays' found in the JSON file.")
    except Exception as e:
        print(f"Error: {e}")

def main():
    parser = argparse.ArgumentParser(description="Read an switch_port")
    parser.add_argument("switch_port", type=int, help="An switch port")

    args = parser.parse_args()
    p4api = SimpleSwitchThriftAPI(
        args.switch_port, "192.168.122.229"
    )  # 9090，127.0.0.1
    clear_tables_and_registers(p4api)

if __name__ == "__main__":
    main()
