import socket
import requests
import yt_dlp  
import os
import threading  
# URL DIR file_name.jpg file_name
def create_directory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

def ensure_extension(filename, extension): 
    if not filename.endswith(extension):
        filename += extension
    return filename

def download_file(url, directory, filename, extension, download_func):
    try:
        create_directory(directory)
        filename = ensure_extension(filename, extension)
        file_path = os.path.join(directory, filename)

        response = requests.get(url, stream=True)
        response.raise_for_status()

        with open(file_path, 'wb') as file:
            for chunk in response.iter_content(chunk_size=1024):
                if chunk:
                    file.write(chunk)

        return f"{download_func} downloaded successfully as {file_path}"
    except Exception as e:
        return f"Error downloading {download_func}: {e}"

def download_image(url, directory, filename):
    return download_file(url, directory or "Images", filename, '.jpg', "Image")

def download_youtube_video(url, directory, filename):
    try:
        directory = directory or "Videos"
        create_directory(directory)
        filename = ensure_extension(filename, '.mp4')
        file_path = os.path.join(directory, filename)


        ydl_opts = {'outtmpl': file_path}
        with yt_dlp.YoutubeDL(ydl_opts) as ydl:
            ydl.download([url])

        return f"Video downloaded successfully as {file_path}"
    except Exception as e:
        return f"Error downloading video: {e}"

def download_audio_from_url(url, directory, filename):
    return download_file(url, directory or "Audio", filename, '.mp3', "Audio")

def download_pdf(url, directory, filename):
    return download_file(url, directory or "PDFs", filename, '.pdf', "PDF")

def download_zip(url, directory, filename):
    return download_file(url, directory or "Zips", filename, '.zip', "ZIP file")

def handle_client(client_socket):
    while True:
        client_socket.send(b"Choose an option:\n1. Download Image\n2. Download Video\n3. Download Audio\n4. Download PDF\n5. Download ZIP\n6. Exit\nEnter your choice: ")
        choice = client_socket.recv(1024).decode().strip()

        if choice == "1":
            result = handle_download(client_socket, "image", download_image)
        elif choice == "2":
            result = handle_download(client_socket, "video", download_youtube_video)
        elif choice == "3":
            result = handle_download(client_socket, "audio", download_audio_from_url)
        elif choice == "4":
            result = handle_download(client_socket, "PDF", download_pdf)
        elif choice == "5":
            result = handle_download(client_socket, "ZIP", download_zip)
        elif choice == "6":
            client_socket.send(b"Goodbye! Disconnecting.\n")
            break
        else:
            result = "Invalid choice. Please try again."

        client_socket.send(result.encode())

        if choice == "6":
            break

    client_socket.close()

def handle_download(client_socket, file_type, download_func):
    client_socket.send(f"Enter {file_type} URL: ".encode())
    url = client_socket.recv(1024).decode().strip()
    client_socket.send(b"Enter directory to save the file (leave blank for default): ")
    directory = client_socket.recv(1024).decode().strip()
    client_socket.send(f"Enter filename to save as (e.g., {file_type}.jpg): ".encode())
    filename = client_socket.recv(1024).decode().strip()

    return download_func(url, directory, filename)

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("127.0.0.1", 9999))
    server.listen(5)
    print("Server listening on port 9999...")

    while True:
        client_socket, addr = server.accept()
        print(f"Accepted connection from {addr}")

        client_thread = threading.Thread(target=handle_client, args=(client_socket,))
        client_thread.start()

if __name__ == "__main__":
    start_server()
