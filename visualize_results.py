import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def plot_benchmarks(csv_file):
    # 1. Đọc dữ liệu
    if not os.path.exists(csv_file):
        print(f"Lỗi: Không tìm thấy file {csv_file}")
        return

    df = pd.read_csv(csv_file)

    # Thiết lập giao diện
    sns.set_theme(style="whitegrid")
    plt.rcParams['figure.figsize'] = (12, 8)

    # 2. Đồ thị 1: So sánh số màu (Result) với Best_K (Số màu tối ưu lý thuyết)
    plt.figure()
    ax = sns.barplot(data=df, x='filename', y='result', hue='algorithm')
    
    # Vẽ đường thẳng ngang cho Best_K của từng file (nếu cần)
    plt.title('So sánh số màu tìm được giữa các thuật toán', fontsize=15)
    plt.ylabel('Số lượng màu (k)')
    plt.xlabel('Dataset')
    plt.xticks(rotation=45)
    plt.legend(title='Thuật toán')
    plt.tight_layout()
    plt.savefig('./grap/color_comparison.png')
    print("Đã lưu đồ thị so sánh màu tại: color_comparison.png")

    # 3. Đồ thị 2: So sánh thời gian thực thi (Time_ms)
    plt.figure()
    sns.lineplot(data=df, x='filename', y='time_ms', hue='algorithm', marker='o')
    plt.yscale('log')  # Dùng thang đo log vì BnB có thể chạy rất lâu so với Tabu/DSATUR
    plt.title('Thời gian thực thi (Thang đo Log ms)', fontsize=15)
    plt.ylabel('Thời gian (ms)')
    plt.xlabel('Dataset')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig('./grap/time_comparison.png')
    print("Đã lưu đồ thị thời gian tại: time_comparison.png")

    # 4. Đồ thị 3: Độ lệch so với Best_K (Match column)
    plt.figure()
    sns.boxplot(data=df, x='algorithm', y='match')
    plt.title('Phân phối độ lệch so với số màu tối ưu (Lower is better)', fontsize=15)
    plt.ylabel('Độ lệch (Result - Best_K)')
    plt.tight_layout()
    plt.savefig('./grap/accuracy_distribution.png')
    print("Đã lưu đồ thị độ chính xác tại: accuracy_distribution.png")

if __name__ == "__main__":
    plot_benchmarks('results.csv')