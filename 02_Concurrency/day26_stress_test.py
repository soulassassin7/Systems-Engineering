import threading
import urllib.request
import time

URL = "http://localhost:8080/"
TOTAL_REQUESTS = 10000 #how many times to hit the server
CONCURRENT_THREADS = 800 #How many users at the exact same time

success_count = 0
fail_count = 0
lock = threading.Lock()

def send_request(i):
	global success_count, fail_count
	try:
		with urllib.request.urlopen(URL,timeout = 5) as response:
			code = response.getcode()
			if code == 200:
				with lock:
					success_count += 1
				# print(f"[{i}] Success")
			else:
				with lock:
					fail_count += 1
				print(f"[{i}] Failed (Status {code})")
	except Exception as e:
		with lock:
			fail_count += 1
		print(f"[{i}] Error: {e}")

def run_test():
	threads = []
	start_time = time.time()
	print(f"--- Starting Stress Test: {TOTAL_REQUESTS} requests ---")
	
	for i in range(TOTAL_REQUESTS):
		t = threading.Thread(target = send_request,args=(i,))
		threads.append(t)
		t.start()
		
		#throttle to prevent crashing the computer (not the server)
		#if len(threads)%CONCURRENT_THREADS == 0:
		#	time.sleep(0.1)

	for t in threads:
		t.join()
	duration = time.time() - start_time
	print(f"\n--- Test Complete in {duration: .2f} seconds ---")
	print(f"Successful: {success_count}")
	print(f"Failed:     {fail_count}")

if __name__ == "__main__":
	run_test()
