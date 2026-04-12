import networkx as nx
import matplotlib.pyplot as plt
import os

def read_testcases(config_file):
    """Đọc danh sách các file cần xử lý từ testcases.txt"""
    enabled_files = []
    if not os.path.exists(config_file):
        print(f"❌ Không tìm thấy file cấu hình: {config_file}")
        return enabled_files

    with open(config_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            parts = line.split(',')
            if len(parts) >= 5:
                filename = parts[0].strip()
                enabled = parts[4].strip()
                if enabled == '1':
                    enabled_files.append(filename)
    return enabled_files

def draw_and_save_graph(filename, input_dir="datasets", output_dir="plots"):
    """Đọc file DIMACS và vẽ đồ thị"""
    path = os.path.join(input_dir, filename)
    if not os.path.exists(path):
        print(f"⚠️ Cảnh báo: Không tìm thấy file dữ liệu tại {path}")
        return

    print(f"🎨 Đang vẽ đồ thị cho: {filename}...")
    G = nx.Graph()
    
    with open(path, 'r') as f:
        for line in f:
            parts = line.split()
            if not parts: continue
            if parts[0] == 'p':
                n = int(parts[2])
                G.add_nodes_from(range(1, n + 1))
            elif parts[0] == 'e':
                u, v = int(parts[1]), int(parts[2])
                G.add_edge(u, v)

    # Tạo thư mục lưu ảnh nếu chưa có
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    plt.figure(figsize=(12, 10))
    
    # Lựa chọn Layout tùy theo quy mô đồ thị
    num_nodes = G.number_of_nodes()
    if num_nodes > 300:
        # Với đồ thị lớn, dùng Circular để tránh rối
        pos = nx.circular_layout(G)
        node_size = 10
        alpha = 0.1
    else:
        # Với đồ thị nhỏ, dùng Spring để thấy rõ cấu trúc
        pos = nx.spring_layout(G, k=0.15, seed=42)
        node_size = 50
        alpha = 0.3

    nx.draw_networkx_nodes(G, pos, node_size=node_size, node_color='teal')
    nx.draw_networkx_edges(G, pos, width=0.5, alpha=alpha, edge_color='black')
    
    plt.title(f"Graph Visualization: {filename}\nNodes: {num_nodes}, Edges: {G.number_of_edges()}")
    plt.axis('off')

    output_path = os.path.join(output_dir, f"{filename}.png")
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close() # Giải phóng bộ nhớ sau khi vẽ
    print(f"✅ Đã lưu: {output_path}")

if __name__ == "__main__":
    # Bước 1: Lấy danh sách file enabled
    files_to_draw = read_testcases("testcases.txt")
    
    if not files_to_draw:
        print("📭 Không có testcase nào được bật (enabled=1).")
    else:
        print(f"🚀 Tìm thấy {len(files_to_draw)} file cần vẽ.")
        # Bước 2: Vẽ từng file
        for f in files_to_draw:
            draw_and_save_graph(f)
        print("\n✨ Hoàn thành tất cả đồ thị!")