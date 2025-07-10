import os
import scipy.io as sio
# This Python file uses the following encoding: utf-8

# if __name__ == "__main__":
#     pass
# 读取 .mat 文件中的 DE 特征
def read_de_features(file):
    data = sio.loadmat(file)
    return data['data']

# 创建包含通道信息的 MNE Info 对象，并映射到标准10-20电极位置
def create_mne_info(channels, sfreq=128):
    info = mne.create_info(channels, sfreq=sfreq, ch_types='eeg')
    montage = mne.channels.make_standard_montage('standard_1020')
    info.set_montage(montage)
    return info

# 将单个频带的 DE 特征转化为 MNE Raw 对象，方便后续分析和可视化
def create_mne_raw_object_for_band(data, info):
    data = data.T  # 将数据转置为 (n_channels, n_times)
    raw = mne.io.RawArray(data, info)
    return raw

# 可视化每个通道的 DE 特征到脑部地形图
def plot_topomap_for_band(raw, band_name):
    # 计算功率谱密度（PSD）
    psd = raw.compute_psd(method='welch', fmin=0, fmax=50)
    # 获取平均功率谱密度
    psd_data = psd.get_data().mean(axis=-1)  # (n_channels,)

    # 使用 mne.viz.plot_topomap 绘制地形图
    fig, ax = plt.subplots(1, 1, figsize=(16, 16))  # 增大图像的尺寸
    im, _ = mne.viz.plot_topomap(psd_data, raw.info, cmap='RdBu_r', sensors=True, show=False)

    # 添加标题
    ax.set_title(f'{band_name} DE Features')

    # 保存图片
    plt.savefig(f'D:/github/deap/Deap_De/{band_name}_topomap.png', dpi=600)  # 提高图像分辨率，设置 dpi

    # 关闭当前图形，以避免多图干扰
    plt.close(fig)

# 主程序
if __name__ == '__main__':
    print("[DEBUG] plot.py 已被調用！")
    # 定义数据路径
    dataset_dir = "D:/github/deap/Deap_De/"
    file = os.path.join(dataset_dir, "DE_s09.mat")

    # 读取 DE 特征
    de_features = read_de_features(file)

    # 定义通道名称（对应 DEAP 数据集中 32 个通道）
    channels = [
        "Fp1", "AF3", "F3", "F7", "FC5", "FC1", "C3", "T7", "CP5", "CP1",
        "P3", "P7", "PO3", "O1", "Oz", "Pz", "Fp2", "AF4", "Fz", "F4",
        "F8", "FC6", "FC2", "Cz", "C4", "T8", "CP6", "CP2", "P4", "P8",
        "PO4", "O2"
    ]

    # 假设数据被分为4个频带，每个频带32个通道
    num_bands = 4
    band_names = ['Theta', 'Alpha', 'Beta', 'Gamma']
    sfreq = 128

    # 创建 MNE Info 对象并映射电极位置
    info = create_mne_info(channels, sfreq=sfreq)

    # 逐个频带处理和可视化
    for i in range(num_bands):
        band_data = de_features[:, i * len(channels):(i + 1) * len(channels)]
        raw = create_mne_raw_object_for_band(band_data, info)
        plot_topomap_for_band(raw, band_name=band_names[i])

    # 将矩阵转换为 numpy 数组，并转换为 PyTorch 张量
    matrixs = {}
    for i in range(num_bands):
        band_data = de_features[:, i * len(channels):(i + 1) * len(channels)]
        matrixs[band_names[i]] = band_data

    x = np.array(list(matrixs.values()), dtype=np.float32)
    x = torch.from_numpy(x)
    x = x.unsqueeze(0)  # 添加批次维度
