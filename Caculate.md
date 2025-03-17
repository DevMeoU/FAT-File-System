**Để đọc và hiểu cấu trúc của hệ thống tệp FAT (FAT12, FAT16, FAT32), bạn cần trích xuất và tính toán một số giá trị quan trọng từ **boot sector**, sau đó sử dụng chúng để xác định vị trí của **root directory**, **sub directories**, và **file data**. Dưới đây là các giá trị cần thiết cùng với cách tính toán để truy cập từng thành phần này.**

---

**1. Các giá trị cần thiết từ Boot Sector**

**Boot sector là sector đầu tiên (sector 0) của hệ thống tệp FAT, chứa các thông tin cơ bản về cấu trúc hệ thống. Bạn cần đọc và trích xuất các giá trị sau:**

* **BytesPerSector**: Số byte trong một sector (thường là 512, 1024, 2048, hoặc 4096).
  * **Offset trong boot sector: 11-12.**
* **SectorsPerCluster**: Số sector trong một cluster.
  * **Offset: 13.**
* **ReservedSectors**: Số sector dự phòng (thường là 1 cho FAT12/FAT16, lớn hơn cho FAT32).
  * **Offset: 14-15.**
* **NumberOfFATs**: Số lượng bảng FAT (thường là 2).
  * **Offset: 16.**
* **RootEntryCount**: Số lượng entry trong root directory (chỉ áp dụng cho FAT12/FAT16).
  * **Offset: 17-18.**
* **TotalSectors**: Tổng số sector trong hệ thống tệp.
  * **Nếu **TotalSectors16** (offset 19-20) bằng 0, dùng **TotalSectors32** (offset 32-35).**
* **FATSize**: Số sector cho mỗi bảng FAT.
  * **Nếu **FATSize16** (offset 22-23) bằng 0, dùng **FATSize32** (offset 36-39, cho FAT32).**
* **RootCluster**: Cluster bắt đầu của root directory (chỉ có trong FAT32).
  * **Offset: 44-47.**

---

**2. Tính toán vị trí của FAT Table**

**Bảng FAT (FAT table) chứa thông tin về trạng thái của các cluster (đã dùng, trống, hoặc lỗi). Vị trí của nó được tính như sau:**

* **Sector bắt đầu của FAT table thứ nhất**:

  ```text
  FirstFATSector = ReservedSectors
  ```
* **Sector bắt đầu của FAT table thứ hai** (nếu **NumberOfFATs > 1**):

  ```text
  SecondFATSector = FirstFATSector + FATSize
  ```

---

**3. Tính toán vị trí của Root Directory**

**Cho FAT12/FAT16**

**Root directory trong FAT12/FAT16 có vị trí và kích thước cố định:**

* **Số sector của root directory**:

  ```text
  RootDirSectors = ((RootEntryCount * 32) + (BytesPerSector - 1)) / BytesPerSector
  ```

  **(Mỗi entry chiếm 32 byte, công thức đảm bảo làm tròn lên).**
* **Sector bắt đầu của root directory**:

  ```text
  FirstRootDirSector = ReservedSectors + (NumberOfFATs * FATSize)
  ```

**Cho FAT32**

**Root directory trong FAT32 không cố định mà là một phần của data area, được xác định bởi **RootCluster** (thường là cluster 2). Cách đọc sẽ tương tự sub directory (xem phần 5).**

---

**4. Tính toán vị trí của Data Area**

**Data area chứa dữ liệu của file và sub directories (bao gồm root directory trong FAT32):**

* **Sector bắt đầu của data area**:
  * **Cho FAT12/FAT16:  **

    ```text
    FirstDataSector = FirstRootDirSector + RootDirSectors
    ```
  * **Cho FAT32:  **

    ```text
    FirstDataSector = ReservedSectors + (NumberOfFATs * FATSize)
    ```

---

**5. Tính toán vị trí của Sub Directories và File Data**

**Sub directories và file được lưu trong các cluster thuộc data area. Để đọc chúng, bạn cần:**

* **Cluster bắt đầu**: Lấy từ entry trong root directory hoặc sub directory cha (32 byte mỗi entry, chứa tên file, thuộc tính, cluster bắt đầu, kích thước).
* **Sector bắt đầu của một cluster**:

  ```text
  FirstSectorOfCluster = FirstDataSector + (clusterNum - 2) * SectorsPerCluster
  ```

  **(Cluster bắt đầu từ 2 trong FAT).**
* **Đọc toàn bộ dữ liệu**:

  * **Dùng bảng FAT để theo dõi chuỗi cluster (cluster chain).  **
  * **Giá trị trong FAT table tại cluster hiện tại chỉ ra cluster tiếp theo.  **
  * **Kết thúc khi gặp giá trị đánh dấu cuối chuỗi:  **
    * **FAT12: **0xFFF
    * **FAT16: **0xFFFF
    * **FAT32: **0xFFFFFFFF

---

**6. Tóm tắt các giá trị và công thức**

**Dưới đây là bảng các giá trị cần thiết để đọc boot sector, root directory, sub directories và file data:**

| **Thành phần**        | **Công thức / Giá trị**                                                                            |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------ |
| **BytesPerSector**      | **Đọc từ boot sector (offset 11-12)**                                                               |
| **SectorsPerCluster**   | **Đọc từ boot sector (offset 13)**                                                                  |
| **ReservedSectors**     | **Đọc từ boot sector (offset 14-15)**                                                               |
| **NumberOfFATs**        | **Đọc từ boot sector (offset 16)**                                                                  |
| **RootEntryCount**      | **Đọc từ boot sector (offset 17-18, cho FAT12/FAT16)**                                              |
| **TotalSectors**        | **Đọc từ offset 19-20 hoặc 32-35**                                                                 |
| **FATSize**             | **Đọc từ offset 22-23 hoặc 36-39**                                                                 |
| **RootCluster**         | **Đọc từ offset 44-47 (cho FAT32)**                                                                 |
| **FirstFATSector**      | **ReservedSectors**                                                                                    |
| **RootDirSectors**      | **((RootEntryCount * 32) + (BytesPerSector - 1)) / BytesPerSector**(cho FAT12/FAT16)                   |
| **FirstRootDirSector**  | **ReservedSectors + (NumberOfFATs * FATSize)**(cho FAT12/FAT16)                                        |
| **FirstDataSector**     | **- FAT12/FAT16:**FirstRootDirSector + RootDirSectors **- FAT32:**ReservedSectors + (NumberOfFATs * FATSize) |
| **Sector của cluster** | **FirstDataSector + (clusterNum - 2) * SectorsPerCluster**                                             |

---

**Kết luận**

**Để đọc hệ thống tệp FAT:**

1. **Trích xuất các giá trị cơ bản từ **boot sector**.**
2. **Tính vị trí của **FAT table**, **root directory** (cho FAT12/FAT16), và **data area**.**
3. **Đọc **root directory** để lấy thông tin file/sub directory (FAT12/FAT16) hoặc dùng **RootCluster** (FAT32).**
4. **Dùng cluster bắt đầu và bảng FAT để truy cập **sub directories** và **file data**.**

**Hy vọng danh sách này giúp bạn hiểu rõ các giá trị cần thiết và cách tính toán để làm việc với hệ thống tệp FAT!**
