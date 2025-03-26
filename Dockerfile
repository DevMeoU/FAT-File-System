# Sử dụng Ubuntu làm base image và cài đặt GCC, g++ và make
FROM ubuntu:latest  

# Cập nhật hệ thống và cài đặt GCC, g++ và make
RUN apt-get update && apt-get install -y gcc g++ make  

# Thiết lập thư mục làm việc
WORKDIR /dock_ws  

# Copy toàn bộ file từ thư mục hiện tại vào container
COPY . /dock_ws  

# Cấp quyền thực thi cho file application.exe (nếu cần)
RUN chmod +x /dock_ws/project/build/bin/application.exe

# Chạy script khi container khởi động (script nằm trong WORKDIR /dock_ws)
CMD ["bash", "DTH_SL.sh"]
