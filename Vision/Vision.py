import numpy as np
import argparse
import imutils
import cv2
import subprocess
import time
from multiprocessing import Process

class Vision:
	

	def __init__ (self):

		#arancione
		self.first_spider_color1 = np.uint8([[[57,114,255]]])
		#verde
		self.first_spider_color2 = np.uint8([[[102,151,49]]])
		#fucsia
		#self.first_spider_color2 = np.uint8([[[180,95,245]]])

		#azzurro
		self.second_spider_color1 = np.uint8([[[216,145,0]]])
		#rosso
		#self.second_spider_color1 = np.uint8([[[78,63,223]]])
		#giallo
		self.second_spider_color2 = np.uint8([[[0,255,255]]])
		

		f_s_c1HSV = cv2.cvtColor(self.first_spider_color1,cv2.COLOR_BGR2HSV)
		self.f_s_c1Lower = np.array((f_s_c1HSV[0][0][0]-10,100,100))
		self.f_s_c1Upper = np.array((f_s_c1HSV[0][0][0]+10,255,255))

		f_s_c2HSV = cv2.cvtColor(self.first_spider_color2,cv2.COLOR_BGR2HSV)
		self.f_s_c2Lower = np.array((f_s_c2HSV[0][0][0]-10,100,100))
		self.f_s_c2Upper = np.array((f_s_c2HSV[0][0][0]+10,255,255))

		s_s_c1HSV = cv2.cvtColor(self.second_spider_color1,cv2.COLOR_BGR2HSV)
		self.s_s_c1Lower = np.array((s_s_c1HSV[0][0][0]-10,100,100))
		self.s_s_c1Upper = np.array((s_s_c1HSV[0][0][0]+10,255,255))

		s_s_c2HSV = cv2.cvtColor(self.second_spider_color2,cv2.COLOR_BGR2HSV)
		self.s_s_c2Lower = np.array((s_s_c2HSV[0][0][0]-10,100,100))
		self.s_s_c2Upper = np.array((s_s_c2HSV[0][0][0]+10,255,255))

		self.camera = cv2.VideoCapture(0)
		fps = self.camera.get(cv2.cv.CV_CAP_PROP_FPS)
		print "Frames per second using video.get(cv2.cv.CV_CAP_PROP_FPS): {0}".format(fps)
		# video recorder
		fourcc = cv2.cv.CV_FOURCC('m', 'p', '4', 'v')
		self.video_writer = cv2.VideoWriter("output.avi", fourcc, 25, (900, 506))
		if not self.video_writer :
			print "!!! Failed VideoWriter: invalid parameters"
			sys.exit(1)

	def close_all(self):
		# cleanup the camera and close any open windows
		self.camera.release()
		self.video_writer.release()
		cv2.destroyAllWindows()

	def get_Spider(self, color1Lower, color1Upper, color2Lower, color2Upper):

		(grabbed, frame) = self.camera.read()
		#frame = cv2.flip(frame,1)
		s_x = None
		s_y = None 
		f_x = None 
		f_y = None
		# if we are viewing a video and we did not grab a frame,
		# then we have reached the end of the video
		if not grabbed:
			print("No frame D:")

		# resize the frame, blur it, and convert it to the HSV
		# color space
		frame = imutils.resize(frame, width=900)
		blurred = cv2.GaussianBlur(frame, (11, 11), 0)
		#cv2.imshow("Blurred", blurred)
		hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
		cv2.imshow("hsv", hsv)

		# construct a mask for the color "green", then perform
		# a series of dilations and erosions to remove any small
		# blobs left in the mask
		mask = cv2.inRange(hsv, color1Lower, color1Upper)
		mask = cv2.erode(mask, None, iterations=3)
		mask = cv2.dilate(mask, None, iterations=2)

		# cv2.imshow("mask1",cv2.bitwise_and(hsv,hsv,mask=mask));

		# find contours in the mask and initialize the current
		# (x, y) center of the ball
		cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)[-2]
		center = None
		# only proceed if at least one contour was found
		if len(cnts) > 0:
			# find the largest contour in the mask, then use
			# it to compute the minimum enclosing circle and
			# centroid
			c = max(cnts, key=cv2.contourArea)
			((f_x, f_y), radius) = cv2.minEnclosingCircle(c)
			M = cv2.moments(c)
			center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

			# only proceed if the radius meets a minimum size
			if radius > 5:
				# draw the circle and centroid on the frame,
				# then update the list of tracked points
				cv2.circle(frame, (int(f_x), int(f_y)), int(radius),
				(0, 255, 255), 2)
				cv2.circle(frame, center, 5, (0, 0, 255), -1)


		mask = cv2.inRange(hsv, color2Lower, color2Upper)
		mask = cv2.erode(mask, None, iterations=3)
		mask = cv2.dilate(mask, None, iterations=2)

		# cv2.imshow("mask2",cv2.bitwise_and(hsv,hsv,mask=mask));

		# find contours in the mask and initialize the current
		# (x, y) center of the ball
		cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)[-2]
		center = None
		# only proceed if at least one contour was found
		if len(cnts) > 0:
			# find the largest contour in the mask, then use
			# it to compute the minimum enclosing circle and
			# centroid
			c = max(cnts, key=cv2.contourArea)
			((s_x, s_y), radius) = cv2.minEnclosingCircle(c)
			M = cv2.moments(c)
			center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

			# only proceed if the radius meets a minimum size
			if radius > 5:
				# draw the circle and centroid on the frame,
				# then update the list of tracked points
				cv2.circle(frame, (int(s_x), int(s_y)), int(radius),
				(255, 255, 0), 2)
				cv2.circle(frame, center, 5, (0, 0, 255), -1)
		
		cv2.imshow("Frame", frame)
		self.video_writer.write(frame)

		if f_x and f_y and s_x and s_y:
			p0 = np.array([f_x,f_y])
			p1 = np.array([s_x,s_y])
			
			points = dict()

			points["p0"] = p0
			points["p1"] = p1

			return [points, self.calculate_matrix(p0,p1)]


	def get_Spider_Inseguitore(self, color1Lower, color1Upper):

		(grabbed, frame) = self.camera.read()
		#frame = cv2.flip(frame,1)
		s_x = None
		s_y = None 
		f_x = None 
		f_y = None
		# if we are viewing a video and we did not grab a frame,
		# then we have reached the end of the video
		if not grabbed:
			print("No frame D:")

		# resize the frame, blur it, and convert it to the HSV
		# color space
		frame = imutils.resize(frame, width=900)
		blurred = cv2.GaussianBlur(frame, (11, 11), 0)
		#cv2.imshow("Blurred", blurred)
		hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
		# cv2.imshow("hsv", hsv)

		# construct a mask for the color "green", then perform
		# a series of dilations and erosions to remove any small
		# blobs left in the mask
		mask = cv2.inRange(hsv, color1Lower, color1Upper)
		mask = cv2.erode(mask, None, iterations=3)
		mask = cv2.dilate(mask, None, iterations=2)

		# cv2.imshow("mask1",cv2.bitwise_and(hsv,hsv,mask=mask));

		# find contours in the mask and initialize the current
		# (x, y) center of the ball
		cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)[-2]
		center = None
		# only proceed if at least one contour was found
		if len(cnts) > 0:
			# find the largest contour in the mask, then use
			# it to compute the minimum enclosing circle and
			# centroid
			c = max(cnts, key=cv2.contourArea)
			((f_x, f_y), radius) = cv2.minEnclosingCircle(c)
			M = cv2.moments(c)
			center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

			# only proceed if the radius meets a minimum size
			if radius > 5:
				# draw the circle and centroid on the frame,
				# then update the list of tracked points
				cv2.circle(frame, (int(f_x), int(f_y)), int(radius),
				(0, 255, 255), 2)
				cv2.circle(frame, center, 5, (0, 0, 255), -1)


		
		cv2.imshow("Frame", frame)
		self.video_writer.write(frame)

		if f_x and f_y:
			p1 = np.array([f_x, f_y])
			return p1

	#dati due punti restituisce la matrice omogenea di rototraslazione
	def calculate_matrix(self, p0, p1, versor=[1,0]):

		vet_diff = p1 - p0
		x_axis = np.array(versor)
		dot_product = np.dot(vet_diff,x_axis)
		module = np.linalg.norm(vet_diff)
		cos_arg = dot_product/module
		angle = np.arccos(cos_arg)

		#print(vet_diff, cos_arg, np.degrees(angle))

		rot_matrix = [cos_arg, -np.sin(angle), np.sin(angle), cos_arg]
		centr_vet = [(p0[0]+p1[0])/2, (p0[1]+p1[1])/2]
		matrix = np.array([rot_matrix[0], rot_matrix[1], centr_vet[0], rot_matrix[2], rot_matrix[3], centr_vet[1], 0 ,0 ,1])
		matrix = np.reshape(matrix, (3,3))
		#print(matrix)
		return matrix


def launch_curl(string):
	subprocess.call("curl -m 1 http://192.168.4.1/?c=m0"+string, shell=True)

if __name__ == '__main__':

	g = Vision()

	stop = True
	old_command = "stop"

	while True:
		returns = g.get_Spider(g.f_s_c1Lower, g.f_s_c1Upper, g.f_s_c2Lower, g.f_s_c2Upper)
		point = g.get_Spider_Inseguitore(g.s_s_c1Lower, g.s_s_c1Upper)

		key = cv2.waitKey(1) & 0xFF
		if key == ord("q"):
			g.close_all()
			print("BYEEEEE")
			break

		if returns is not None and point is not None and returns[0] is not None and returns[1] is not None:
			#print(first_matrix, second_matrix)

			#print(first_matrix[0][0])

			print (old_command)
			points = returns[0]
			first_matrix = returns[1]

			p0 = [first_matrix[0][2], first_matrix[1][2]]
			p1 = point

			new_matrix_x= g.calculate_matrix(np.array(p0), p1)
			new_matrix_y= g.calculate_matrix(np.array(p0), p1, [0,1])

			cos_x = new_matrix_x[0][0]
			cos_y = new_matrix_y[0][0]

			#print("Coseno rispetto X: "+ str(cos_x))
			#print("Coseno rispetto Y: "+ str(cos_y))

			#epsilon di rotazione del robot rispetto alla telecamera
			epsilon_rot_robot = 0.8
			#epsilon di rotazione dell'avversario rispetto al robot
			epsilon_rot_enemy = 0.5
			#epsilon di rotazione dell'avversario rispetto al robot rispetto all'asse y
			epsilon_fron = 0.5
			#epsilon di stop
			epsilon_stop = 230

			diff = np.linalg.norm(p1-p0, ord = 2)

			#print(diff)
			if diff < epsilon_stop:
				if old_command != "stop":
					old_command = "stop"
					print("FERMATIIIIIIIII")
					p = Process(target=launch_curl,args=('128128', ))
					p.start()
					
				continue

			if abs(first_matrix[0][0]) <= epsilon_rot_robot:

				diff = points["p0"] - points["p1"]
				
				if diff[1] < 0:
					#print("Arancione Davanti")

					if cos_x > epsilon_rot_enemy:
						print("Vai a Destra__Verticale__Arancione")
						if old_command != "right":
							old_command = "right"
							p = Process(target=launch_curl,args=('000255', ))
							p.start()
							continue
					elif cos_x < -epsilon_rot_enemy:
						print("Vai a Sinistra__Verticale__Arancione")
						if old_command != "left":
							old_command = "left"
							p = Process(target=launch_curl,args=('255000', ))
							p.start()
							continue
					
					if cos_y > epsilon_fron:
						print("Vai a Indietro__Verticale__Arancione")
						if old_command != "back":
							old_command = "back"	
							p = Process(target=launch_curl,args=('000000', ))
							p.start()
							continue
					elif cos_y < -epsilon_fron:
						print("Vai a Avanti__Verticale__Arancione")
						if old_command != "front":
							old_command = "front"
							p = Process(target=launch_curl,args=('255255', ))
							p.start()
							continue
				else:
					#print("Verde Davanti")
					if cos_x > epsilon_rot_enemy:
						if old_command != "left":
							old_command = "left"
							print("Vai a Sinistra__Verticale__Arancione")
							p = Process(target=launch_curl,args=('255000', ))
							p.start()
							continue
					elif cos_x < -epsilon_rot_enemy:
						if old_command != "right":
							old_command = "right"
							print("Vai a Destra__Verticale__Arancione")
							p = Process(target=launch_curl,args=('000255', ))
							p.start()
							continue
					
					if cos_y > epsilon_fron:
						if old_command != "front":
							old_command = "front"
							p = Process(target=launch_curl,args=('255255', ))
							p.start()
							print("Vai a Avanti__Verticale__Arancione")
							continue
					elif cos_y < -epsilon_fron:
						if old_command != "back":
							old_command = "back"
							p = Process(target=launch_curl,args=('000000', ))
							p.start()
							print("Vai a Indietro__Verticale__Arancione")
							continue
			else:
				diff = points["p0"] - points["p1"]
				
				if diff[0] < 0:
					#print("Arancione Sinistra")
					if cos_x > epsilon_rot_enemy:
						if old_command != "back":
							old_command = "back"
							print("Vai a Indietro__Orizzontale__Arancione")
							p = Process(target=launch_curl,args=('000000', ))
							p.start()
							continue
					elif cos_x < -epsilon_rot_enemy:
						if old_command != "front":
							old_command = "front"
							print("Vai a Avanti__Orizzontale__Arancione")
							p = Process(target=launch_curl,args=('255255', ))
							p.start()
							continue
					
					if cos_y > epsilon_fron:
						if old_command != "left":
							old_command = "left"
							p = Process(target=launch_curl,args=('255000', ))
							p.start()
							print("Vai a Sinistra__Orizzontale__Arancione")
							continue
					elif cos_y < -epsilon_fron:
						if old_command != "right":
							old_command = "right"
							p = Process(target=launch_curl,args=('000255', ))
							p.start()
							print("Vai a Destra__Orizzontale__Arancione")
							continue
				else:
					#print("Verde Sinistra")
					if cos_x > epsilon_rot_enemy:
						if old_command != "front":
							old_command = "front"
							print("Vai a Avanti__Orizzontale__Verde")
							p = Process(target=launch_curl,args=('255255', ))
							p.start()
							continue
					elif cos_x < -epsilon_rot_enemy:
						if old_command != "back":
							old_command = "back"
							print("Vai a Indietro__Orizzontale__Arancione")
							p = Process(target=launch_curl,args=('000000', ))
							p.start()
							continue
					
					if cos_y > epsilon_fron:
						if old_command != "right":
							old_command = "right"
							p = Process(target=launch_curl,args=('000255', ))
							p.start()
							print("Vai a Destra__Orizzontale__Arancione")
							continue
					elif cos_y < -epsilon_fron:
						if old_command != "left":
							old_command = "left"
							p = Process(target=launch_curl,args=('255000', ))
							p.start()
							print("Vai a Sinistra__Orizzontale__Arancione")
							continue


		else:
			print("Missing one of the components")
			if old_command != "stop":
				old_command = "stop"
				p = Process(target=launch_curl,args=('128128', ))
				p.start()
			continue






