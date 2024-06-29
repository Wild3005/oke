import cv2
import os

def capture_frames(output_folder):
    # Membuat folder output jika tidak ada
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Membuka kamera
    video = cv2.VideoCapture(0)  # 0 adalah indeks default untuk webcam pertama
    count = 0

    if not video.isOpened():
        print("Error: Could not open webcam.")
        return

    while True:
        success, frame = video.read()
        if not success:
            print("Failed to capture image")
            break

        # Menyimpan frame sebagai file gambar
        frame_path = os.path.join(output_folder, f"frame_{count:04d}.jpg")
        cv2.imwrite(frame_path, frame)
        count += 1

        # Menampilkan frame yang sedang ditangkap
        cv2.imshow('Frame', frame)

        # Menekan 'q' untuk keluar dari loop
        if cv2.waitKey(100) & 0xFF == ord('q'):
            break

    video.release()
    cv2.destroyAllWindows()
    print(f"Captured {count} frames to '{output_folder}'")

if __name__ == "__main__":
    output_folder = "../frame"  # Ganti dengan folder tujuan
    capture_frames(output_folder)
