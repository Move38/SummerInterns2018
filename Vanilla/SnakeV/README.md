@@ -6,3 +6,67 @@ Shiyun Liu is a game designer and game developer, working as game design intern
## Stuff in this repo
1. Snake game
2. Using Sublime & Arduino & Stino


Core Features:
1. Snake moving
2. Snake passing (from one blinks to another), with direction reveresd each time
3. Apple generator and self-destroy
4. Snake length increases when the head eats apple
5. Snake length decreases when pass to empty(in development)
6. Speed up when keeping running in a single Blinks
7. Speed down when length decreases and reset when transfer to a new Blinks
8. Out game UI cycle: 
	READY ----------> GAMEPLAY -> GAMEOVER ----------> RESET -> READY
          single-click                     double-click


Details of each feature:
1. Snake moving:
	Each blinks have a int array to store the snake part of each face
		for example, numSnakeArray[f] is the snake part on the face f
		if it's the first one - the head, it will be 1; so and so forth
		if there is no snake body at the face f, numSnakeArray[f] = 0
		If there is an apple at the face f, numSnakeArray[f] = APPLE
		Every frame, use UpdateSnakeArray() to update the array, push each number forward

		drawFace():
		Every frame, drawFace will draw each face according to the number
		
2. Snake passing:
	passToFace(through which the snake is passing to another Blinks,
	passFromFace( from which the snake is passing to current Blinks),
	snakeFace(the face where head(index: 1) is)
	The default is 6, which is a IMPOSSIBLE_INDEX

	Sender: if head is in current piece,(!=IMPOSSIBLE_INDEX)
	if single-click, 
		snake pass to another blinks by setting a value at the passToFace
	else 
		move forward every 250ms

	Receiver: else
	receive message(no longer update every 250ms)

	MessageMode:
		DATA: 
			Sender: send the data about snake length and direction
			Receiver: receive the data, then init the head and timer
		Receiver:

	UPDATE: send the snake part should be updated and (tell if need to update length at the same time)

3. Apple generator and self-destroy
	Apple only generate when head is not in the current Blinks,
	Apple generate randomly,
	Apple should not cover the snake body
	Only one apple will be in a single piece temporarily
	Apple will self-destroy after a specific time(can be random in the future)

4. Snake length increase when head hit an apple
	It will update snakeLength to all the pieces and find the last one, add a tail after that

5. Snake length decreases when pass to empty
	It will update snakeLength to all the pieces and find the last one

6. Speed up when keeping running in a single Blinks
	Use numMovesHere to count how many moves the snake in current Blinks; reset to 0 when the snake passes to another Blinks
	The gap edcreases every round by 20, the minimum is 150

7. When losing a tail, the gap increases by 20, the maximum is 250,









