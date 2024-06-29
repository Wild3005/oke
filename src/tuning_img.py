import cv2
import numpy as np

def empty(a):
    pass

# Membuat jendela dan trackbar untuk mengatur nilai HSV
cv2.namedWindow("Range HSV")
cv2.resizeWindow("Range HSV", 500, 350)
cv2.createTrackbar("HUE Min", "Range HSV", 0, 180, empty)
cv2.createTrackbar("HUE Max", "Range HSV", 180, 180, empty)
cv2.createTrackbar("SAT Min", "Range HSV", 0, 255, empty)
cv2.createTrackbar("SAT Max", "Range HSV", 255, 255, empty)
cv2.createTrackbar("VALUE Min", "Range HSV", 0, 255, empty)
cv2.createTrackbar("VALUE Max", "Range HSV", 255, 255, empty)

# Baca gambar
img = cv2.imread('../biru.jpg')

while True:
    frame = img.copy()  # Salin gambar asli agar tidak merusaknya

    # Terapkan filter untuk mengurangi noise
    # frame = cv2.bilateralFilter(frame, 20, 20*2, 20/2)

    # Dapatkan nilai dari trackbar
    h_min = cv2.getTrackbarPos("HUE Min", "Range HSV")
    h_max = cv2.getTrackbarPos("HUE Max", "Range HSV")
    s_min = cv2.getTrackbarPos("SAT Min", "Range HSV")
    s_max = cv2.getTrackbarPos("SAT Max", "Range HSV")
    v_min = cv2.getTrackbarPos("VALUE Min", "Range HSV")
    v_max = cv2.getTrackbarPos("VALUE Max", "Range HSV")

    lower_range = np.array([h_min, s_min, v_min])
    upper_range = np.array([h_max, s_max, v_max])

    # Konversi gambar ke ruang warna HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Terapkan threshold dengan inRange
    thresh = cv2.inRange(hsv, lower_range, upper_range)
    bitwise = cv2.bitwise_and(frame, frame, mask=thresh)

    # Temukan kontur
    contours, hierarchy = cv2.findContours(thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    for pic, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if area > 600:
            x, y, w, h = cv2.boundingRect(contour)
            imageFrame = cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 2)
            cv2.putText(imageFrame, "Detected Object", (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

    # Ubah ukuran frame untuk ditampilkan
    resized_frame = cv2.resize(frame, (0, 0), fx=0.1, fy=0.1)
    resized_gray = cv2.resize(gray, (0, 0), fx=0.1, fy=0.1)
    resized_hsv = cv2.resize(hsv, (0, 0), fx=0.1, fy=0.1)
    resized_thresh = cv2.resize(thresh, (0, 0), fx=0.1, fy=0.1)
    resized_bitwise = cv2.resize(bitwise, (0, 0), fx=0.1, fy=0.1)

    cv2.imshow('Frame', resized_frame)
    cv2.imshow('Gray', resized_gray)
    cv2.imshow('HSV', resized_hsv)
    cv2.imshow('Thresh', resized_thresh)
    cv2.imshow('Bitwise', resized_bitwise)

    k = cv2.waitKey(1) & 0xFF
    if k == 27:  # Tekan tombol Esc untuk keluar
        break

cv2.destroyAllWindows()
