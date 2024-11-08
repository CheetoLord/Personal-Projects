def getChoice(choices):
	choices = [str(i) for i in range(1, choices+1)]
	choice = input("Your choice: ")
	while(choice not in choices):
		choice = input("Please input the number of your choice: ")
	return choice


def secondDoorLeftDie():
	print("You burst through the door on your left.")
	print("You close the door behind you, only to be left in total darkness.")
	print("You frantically scramble for a light switch.")
	print("Once you find it, you flick it and wheel around.")
	print("You are now staring down the barrel of a tank.")
	print("It shoots you. You die.")

def secondDoorRightDie():
	print("You rush through the door on the right,")
	print("only to be greeted by a suspiciously empty rooom.")
	print("You start hearing cracks above you, and looking up,")
	print("you see that something is about to crash through the ceiling.")
	print("Before you can react, a tank falls on top of you.")
	print("You are instantly flattened. You die.")

def windowChoice():
	print("Not trusting whoever has put you in this place,")
	print("You try to open the window.")
	print("Unfortunately, it is locked.")
	print("Undetered, you back up, and throw yourself through the window.")
	print("This makes a loud noise, and you know you have to act fast.")
	print("The new large room you have found yourself in has two doors,")
	print("As well as what looks to be a control panel of sorts to your right.")
	print("What do you do?")
	print("\t1: Try the left door")
	print("\t2: Try the right door")
	print("\t3: Mess with the control panel")
	choice = getChoice(3)
	print("\n\n")
	if(choice == "1"):
		secondDoorLeftDie()
	elif(choice == "2"):
		secondDoorRightDie()


def firstDoorDie():
	print("You turn the doorknob, and you see that it is unlocked.")
	print("You swing the door open.")
	print("You are now staring straight down the barrel of a tank.")
	print("It shoots you. You die.")

def firstDoNothingDie():
	print("You lie back down.")
	print("Suddenly, a tank bursts through the wall behind you.")
	print("It runs over you. You die.")

def start():
	print("You wake up in an unfamiliar house")
	print("You see a door in front of you.")
	print("There is also a window on your right.")
	print("What do you do?")
	print("\t1: Try the door")
	print("\t2: Try the window")
	print("\t3: Lie back down, and hope for the best")
	choice = getChoice(3)
	print("\n\n")
	if(choice == "1"):
		firstDoorDie()
	elif(choice == "3"):
		firstDoNothingDie()
	elif(choice == "2"):
		windowChoice()

def main():
	start()
	offerRetry()


def offerRetry():
	print()
	choice = input("Would you like to play again? (y or n):").lower()
	while(choice != "n" and choice != "y"):
		print("Please type either 'n' for no, or 'y' for yes.")
		choice = input("Would you like to play again? (y or n):").lower()
	print("\n\n")
	if(choice == "y"):
		main()
	elif(choice == "n"):
		print("Thanks for playing!")

if(__name__ == "__main__"):
	main()