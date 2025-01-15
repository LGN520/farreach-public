import paramiko
from scp import SCPClient
import re
import sys
import time
def ssh_connect(host, port, username, password):
    """
    创建 SSH 连接
    
    参数:
        host (str): 服务器地址
        port (int): SSH 端口号
        username (str): SSH 用户名
        password (str): SSH 密码

    返回:
        SSHClient: 活跃的 SSH 客户端连接
    """
    try:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=host, port=port, username=username, password=password)
        return client

    except Exception as e:
        print(f"Error connecting to {host}: {str(e)}")
        return None

def execute_command(client, command, working_directory=None):
    """
    在远程服务器上执行命令

    参数:
        client (SSHClient): SSH 客户端连接
        command (str): 要执行的命令
        working_directory (str): 命令执行的工作目录

    返回:
        str: 命令输出
    """
    try:
        if working_directory:
            command = f"cd {working_directory} && {command}"
        stdin, stdout, stderr = client.exec_command(command)
        output = stdout.read().decode()
        error = stderr.read().decode()
        if error:
            print(f"Error executing command '{command}': {error}")
        return output
    except Exception as e:
        print(f"Error executing command '{command}': {str(e)}")
        return ""

def filter_statistics(output):
    """
    筛选输出中以 [Statistics] 开头的行，并提取每行的第一个浮点数

    参数:
        output (str): 原始输出

    返回:
        tuple: (过滤后的输出, 提取的浮点数列表)
    """
    filtered_lines = [line for line in output.splitlines() if line.startswith("[Statistics]")]
    filtered_output = '\n'.join(filtered_lines)

    # 提取每行的第一个浮点数
    floats = []
    for line in filtered_lines:
        match = re.search(r"[+-]?\d*\.\d+", line)
        if match:
            floats.append(float(match.group()))

    return filtered_output, floats

def transfer_file(client, local_path, remote_path):
    """
    使用 SCP 将文件从本地传输到远程服务器

    参数:
        client (SSHClient): SSH 客户端连接
        local_path (str): 本地文件路径
        remote_path (str): 远程文件路径
    """
    try:
        with SCPClient(client.get_transport()) as scp:
            scp.put(local_path, remote_path)
        print(f"File {local_path} transferred to {remote_path}")
    except Exception as e:
        print(f"Error transferring file: {str(e)}")
        
Distreach_home = "/home/jz/farreach-private/distreach"
iterations = 76

def main():
    # Server configurations
    server0 = {
        "host": "10.26.43.53",
        "port": 43053,
        "username": "jz",
        "password": "t9r57oh24i",
    }

    server1 = {
        "host": "10.26.43.163",
        "port": 43060,
        "username": "jz",
        "password": "kjy27st9rj",
    }
    # Number of iterations
    # iterations = 76
    # Step 1: Connect to both servers
    client0 = ssh_connect(**server0)
    client1 = ssh_connect(**server1)

    if not client0 or not client1:
        print("Failed to connect to one or both servers.")
        return

    try:
        load_time = 0.0
        persist_time = 0.0
        sum_switch_time = 0.0
        print("Executing ./simu_recover_load_persist on server0...")
        output = execute_command(client0, "./simu_recover_load_persist 0",Distreach_home)
        filtered_output, floats = filter_statistics(output)
        print(filtered_output)
        print(f"Extracted floats: {floats}")
        load_time += floats[1]
        persist_time += floats[2]
        print("Executing ./simu_recover_load_persist on server1...")
        output = execute_command(client1, "./simu_recover_load_persist 1",Distreach_home)
        filtered_output, floats = filter_statistics(output)
        print(filtered_output)
        print(f"Extracted floats: {floats}")
        load_time += floats[1]
        persist_time += floats[2]
        for i in range(0,iterations):
            print(f"Iteration {i + 1}...")
            # Step 4: Execute ./simu_recover_load_persist on server0 and server1
            switch_time = 0
            # Step 5: Execute /simu_failure on server0
            print("Executing ./simu_failure on server0...")
            execute_command(client0, "./simu_failure",Distreach_home)
            time.sleep(30)
            # Step 6: Execute ./simu_recover_switch on server0
            print("Executing ./simu_recover_switch on server0...")
            output = execute_command(client0, "./simu_recover_switch",Distreach_home)
            filtered_output, floats = filter_statistics(output)
            print(filtered_output)
            print(f"Extracted floats: {floats}")
            switch_time = floats[1]
            sum_switch_time += switch_time
            sys.stdout.flush()
            time.sleep(60)
        print("[simulate]",iterations,"switched crash, time load",load_time,"switch",sum_switch_time,"server",persist_time)

    finally:
        # Close connections
        client0.close()
        client1.close()

if __name__ == "__main__":
    # get itertation for args
    if len(sys.argv) > 1:
        iterations = int(sys.argv[1])
    main()
