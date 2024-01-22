# 打开输入文件和输出文件
input_file = 'egress_mat.p4'  # 输入文件名
output_file = 'egress_mat.p4.out'  # 输出文件名

def process_line(line):
    # 查找行中是否包含"item.apply();"格式的文本
    if "apply(" in line:
        # 提取item并进行转换
        item = line.split("")[1].split(".apply();")[0]
        new_line = line.replace(f"{item});", f"{item}.apply(.apply();")
        return new_line
       # 查找行中是否包含"item.setValid();"格式的文本
    elif "add_header(" in line:
        item = line.split("")[1].split(".setValid();")[0]
        new_line = line.replace(f"{item});", f"hdr.{item}.setValid(.setValid();")
        return new_line
    # 查找行中是否包含"remove_header(item);"格式的文本
    elif "remove_header(" in line:
        item = line.split("remove_header(")[1].split(");")[0]
        new_line = line.replace(f"remove_header({item});", f"hdr.{item}.setInvalid();")
        return new_line
    # 查找行中是否包含"modify_field(item1, item2);"格式的文本
    elif "modify_field(" in line:
        items = line.split("modify_field(")[1].split(");")[0].split(", ")
        new_line = line.replace(f"modify_field({items[0]}, {items[1]});", f"hdr.{items[0]} = hdr.{items[1]};")
        return new_line
    else:
        return line

# 打开输入文件和输出文件
with open(input_file, "r") as input_file, open(output_file, "w") as output_file:
    # 逐行读取输入文件并处理每一行
    for line in input_file:
        processed_line = process_line(line)
        # 写入处理后的行到输出文件，并保留原有的缩进
        output_file.write(processed_line)
