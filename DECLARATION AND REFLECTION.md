# Assignment Declaration and Reflection

## 1. Integrity Pledge and Learning Intent

By submitting this file, I certify that the work in this assignment is my own original work. I understand that I am fully responsible for all content, code, and explanations submitted. I have documented all sources of assistance, including human collaborators, traditional resources, and Generative AI tools.

**Intent pledge.** Before starting this assignment, my intention for using (or not using) AI tools to support my learning was:

- [x] I plan to use GenAI as an interactive tutor/study aid.
- [ ] I plan to use GenAI only for debugging or syntax help.
- [ ] I do not plan to use GenAI for this assignment.

**How I intended GenAI to support my learning**

I use the same process on every project, including this procedural 3D robotic arm homework. I start by working through the problem myself. I write down what the program has to do, then I research it: course material, online sources, Stack Overflow, and other GitHub repos. I do not ask GenAI to explain the assignment until I already know what I am building and how I want to build it.

After that, GenAI supports the work I already started. I give my notes to Grok model in Cursor IDE, and ask it to turn those notes into a build plan. I then apply that plan myself: I write the code as far as I can from my own notes. I use GenAI again only to double-check what I wrote, not to replace that step. This makes the process fast, efficient and a turns the problem into good learning approach.

So the intent is that I own the understanding and the code. GenAI is there to organize a plan from my notes and to check my work afterward.

> Failure to disclose the actual use of GenAI tools constitutes a breach of academic integrity and will be handled under Academic Misconduct procedures.

## 2. Human Collaborators

List names of any students you discussed high-level logic with, or TAs/instructors who assisted you.

**Sources**

My tutorial TA, Katherine Hou, assisted me on how to approach this problem. I then found a solution based on that:

1. Generate the arm geometry procedurally.
2. Build the kinematic chain and joint interactions.
3. Set up the camera.
4. Add texturing and Phong lighting.

## 3. Traditional Resources

List any documentation, textbooks, or reference sites used (e.g., cppreference.com, language specs, course notes).

**Sources**

- [WebGL Phong shading demo](https://www.cs.toronto.edu/~jacobson/phong-demo/)
- [OpenGL robot arm kinematics 3D simulation demo](https://www.reddit.com/r/opengl/comments/l6hz19/opengl_robot_arm_kinematics_3d_simulation_demo_2/)
- [ResearchGate search for procedural 3D robotic arm kinematics and lighting](https://www.researchgate.net/search.Search.html?query=Procedural+3D+Robotic+Arm+Kinematics+%26+Lighting&type=publication)
- [AI-Driven Virtual Robotic Arm Simulation via OpenCV and Inverse Kinematics](https://www.researchgate.net/publication/404677691_AI-Driven_Virtual_Robotic_Arm_Simulation_via_OpenCV_and_Inverse_Kinematics)

## 4. Generative AI Process and Reflection

Usage of GenAI must be explicitly documented and reflected upon. Credit will only be given for work that is demonstrably your own.

**Did you use Generative AI for this assignment?** Yes

### a. Tools and versions used

Grok 4.7 High on Cursor IDE.

### b. Actual use vs. intent

Describe how you actually used the tool(s). Did your usage align with your initial learning intent? State specific prompts or use-cases (e.g., explaining error messages, brainstorming edge cases, reviewing conceptual understanding).

Yes, my usage aligned with my initial learning intent. I used the GenAI exactly as I mentioned in the "State briefly how you intended GenAI to support your learning"

**Prompts**

- Check the extra draw. I put a declaration where the definition goes, so it will not compile. I am confused about where each piece goes. Can you advice me and how to solve this issue and do not change my comments in mesh.h.
- Update the traditional sources and fix the spelling in the declaration file name. Keep my wordings, just fix the structure of the markdown file.
- For the unit shapes, tell me what to change and where. Give me the idea in the chat. I will write it myself.
- Test geometry.cpp, and fix the indentation in the file.
- Do I need the checkerboard and marble images? I clean the starter files so this folder is only Homework 3. Can you verify this change?
- Add empty robot.cpp and robot.h so I can set up CMakeLists.txt. Do not write the arm code in them.
- Check my implementation of the joint controls. I think I messed up the coordinates for that. Poll the keys every frame instead of using key events. Clamp the pitch joints and let the base spin freely.
- I changed the grippers functions. Previously they opened outward and passed through each other when they close. I let them open past parallel for a wide part, and stoped the close before the tips cross. Can you verify this change please. 
- For the textures, can you explain me the build plan. I will write the disc and the two cylinder meshes myself. Then check robot.cpp, because the draws still called the old cylinder.

**Use cases:** explaining debugging issues, brainstorming, planning, reviewing concep understanding.

### c. Critical evaluation

How did you verify the accuracy of the outputs generated by the AI? Describe any instances where the tool gave incorrect, suboptimal, or misleading guidance, and how you corrected it.

I verified the outside my building/compiling and testing the code again and again and tested it in real time on whatever part the GenAI worked on.

I know the requirements and what exactly i am building which made the testing easy.

GenAI gave me the wrong outputs for the draw function. It gave a code segment in the chat which put a declaration where a definition goes, and i tested it and fixed it myself.

The cylinder caps were another one. The fans already wound opposite each other, and the comment said that made them face outward, but both fans were flipped relative to the wall. The normals were right and culling was off, so the caps still drew and the bug stayed hidden. I swapped the last two indices in each fan, turned on face culling, and checked that the caps stayed visible. If they had vanished, the swap would have been the wrong pair.

The gripper close limit was another one. 24 degrees still let the fingertips pass through each other, because that number followed the centre of each finger and ignored the 0.07 thickness. The inner faces meet near 22.6 degrees, so the clamp is -22. I closed the jaw on screen and the tips touch without crossing.

The first point-light upload did not compile. The vector type was misspelled, the light was sent with the matrix setter, and the draw calls never passed the light. I had that corrected and rebuilt. The window staying open means the shader found the uniform.

### d. Influence on final submission and learning

Reflect on how using (or intentionally avoiding) GenAI influenced your final submission. How did this tool impact your understanding of the core concepts in this assignment compared to working through them without AI assistance?

As mentioned above, I always dedude the problem and solution myself. Even without GenAI, since highschool, I am using GitHub repos and different stack overflow pages which really clears my understanding of different concepts.

Without, GenAI, the work would be a bit slower and i would spend a lot of time on formatting and finding small problems such as fixing the spelling mistakes which is blocking my code from compiling.
