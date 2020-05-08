#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>

int main(int argc, char** argv){

    printf(">> Welcome to testing our version of git\n");
    sleep(3);
    printf(">> We will use the sleep function to show our files change and how they are added and deleted over time\n");
    sleep(3);
    printf(">> We will show one command at a time and pause between each one so you have time to see what happened\n");
    sleep(3);
    printf(">> Lets begin by creating by intializing a server and two client directory which will act like two seperate people commiting and pulling files\n");
    sleep(5);
    system("mkdir server client1 client2");
    system("cp ./WTFserver ./server");
    system("cp ./temp/WTF ./client1");
    system("cp ./temp/WTF ./client2");
    printf(">> The server, client1, client2 directories were created...please open the directories to see how each command affects the files\n");
    sleep(5);
    printf(">> We will first test the configure command\n");
    sleep(3);
    printf(">> The client needs to know the server IP address but for testing purposes we will use 127.0.0.1 and port number 32564\n");
    sleep(5);
    printf(">> In 5 seconds, client1 will run \"./client1/WTF configure 127.0.0.1 32564\", followed by client2 a second later\n");
    sleep(5);
    system("./client1/WTF configure 127.0.0.1 32564");
    sleep(1);
    system("./client2/WTF configure 127.0.0.1 32564");
    printf(">> Look to see the .configure file in client1 and client2 to inspect the result of the command, this program will pause for 20 seconds so you can inspect\n");
    sleep(20);
    printf(">> Ready to move onto the next command?\n");
    sleep(3);
    printf(">> From now on we will turn on the server on port 32564 for the rest of the command\n");
    sleep(5);
    pid_t server;
    if ((server = fork()) == 0){
        //In the child process, exec and replace process with server
        char* args[] = {"./server/WTFserver", "32564", NULL};
        execv("./server/WTFserver", args);
    }
    //system("./server/WTFserver 32564");
    //sleep(3);
    printf(">> The server is now on and listening for commands\n");
    sleep(3);
    printf(">> Lets test create now\n");
    sleep(3);
    printf(">> Client1 will attempt to connect to the server and create project1\n");
    sleep(4);
    system("./client1/WTF create project1");
    printf(">> Inspect client1 and server to see that project1 was created and .Manifest and .history files were intialized and the project was sent to client1\n");
    sleep(10);
    printf(">> We will give 20 seconds to inspect the files and notice that client1 got project1 with the .Manifest but no .history, this is because only the server project keeps track of the history\n");
    sleep(20);
    printf(">> Lets demonstate the \"add\" command by first creating a couple files inside client 1\n");
    
    //Creates text files inside client1 project1
    system("touch ./client1/project1/example.txt");
    system("echo 'random text in here' > ./client1/project1/example.txt");
    system("mkdir ./client1/project1/subdir");
    system("touch ./client1/project1/america.txt");
    system("echo 'Is the best country in the world' > ./client1/project1/america.txt");
    system("touch ./client1/project1/subdir/shakespeare.txt");
    system("echo 'Once upon a time there was a Romeo and a Juliet' > ./client1/project1/subdir/shakespeare.txt");

    printf(">> If you look inside client1's project1 we have conveniently created a bunch of files which we will add to the manifest\n");
    sleep(4);
    printf(">> But first we will give 15 seconds if you want to inspect these files\n");
    sleep(15);
    
    //Run add command for example.txt in client1 project1
    printf(">> In 5 seconds we will run \"./client1/WTF add project1 ./client1/project1/example.txt\" to demonstrate the add command\n");
    sleep(5);
    system("./client1/WTF add project1 ./client1/project1/example.txt");
    printf(">> Just added the file to client1 project1's manifest...we will give 20 seconds to see the results\n");
    sleep(20);

    //Run add command for america.txt in client1 project1 but dont use relative path
    printf(">> Did you see how the file was added to the manifest? Pretty cool\n");
    sleep(5);
    printf(">> Notice how in the add command for the filename we used the relative file path\n");
    sleep(4);
    printf(">> We aren't just limited to that though...if the file we are trying to add is immediately under the project name we can just use the filename\n");
    sleep(8);
    printf(">> We will demonstrate that it works by running \"./client1/WTF add project1 america.txt\" in 5 seconds\n");
    sleep(5);
    system("./client1/WTF add project1 america.txt");
    printf(">> Just added america.txt to the manifest, notice how we just gave the filename and not the path, we will give 15 seconds to inspect\n");
    sleep(15);
    printf(">> When running the add command for yourself note that you can only use the filename ONLY IF the file is immediately under the project name\n");
    sleep(8);
    printf(">> If the file is in a subdirectory in the project you MUST use the filepath\n");
    sleep(6);

    //Run add command for shakespeare.txt
    printf(">> Lets add shakespeare.txt which is in a subdirectory inside project1\n");
    sleep(4);
    printf(">> In 5 seconds we will run \"./client1/WTF add project1 ./client1/project1/subdir/shakespeare.txt\"\n");
    sleep(5);
    system("./client1/WTF add project1 ./client1/project1/subdir/shakespeare.txt");
    printf(">> Just added shakespeare, we will give 15 seconds to see the reuslts\n");
    sleep(15);

    //Run remove command for america.txt
    printf(">> Now lets demonstrate the remove command by removing america.txt from the manifest\n");
    sleep(5);
    printf(">> In 5 seconds we will run \"./client1/WTF remove project1 america.txt\"\n");
    sleep(5);
    system("./client1/WTF remove project1 client1/project1/america.txt");
    printf(">> Just removed america.txt we will give 15 seconds to see the results\n");
    sleep(3);
    printf(">> Notice how the same rules apply to the filename in remove as they did in add\n");
    sleep(15);

    //Add america.txt back in
    printf(">> Good? Lets move on\n");
    sleep(2);
    printf(">> Lets just add america.txt back in then move on to see how commiting and pushing works\n");
    system("./client1/WTF add project1 client1/project1/america.txt"); //Deleted
    sleep(6);

    //Demonstrate commit
    printf(">> Lets demonstrate how commit works\n");
    sleep(3);
    printf(">> In 5 seconds we will run \"./client1/WTF commit project1\"\n");
    sleep(5);
    system("./client1/WTF commit project1");
    printf(">> The project was just committed, there should be a .Commit file inside project1\n");
    sleep(4);
    printf(">> Notice that since project1 is empty on the server, every file we are commit will use the 'A' command\n");
    sleep(5);
    printf(">> We will show that 'M' and 'D' command both work when we deal with client2\n");
    sleep(4);
    printf(">> For now inspect the .Commit file to see that we are adding all 3 files\n");
    printf(">> Also see that there is an 'active commit' in the server repo\n");
    sleep(4);
    printf(">> The server repo's active commit's naming convention is <projectname>_<host>\n");
    sleep(4);
    printf(">> The underscore host indentifies which client's repository the commit belongs to\n");
    sleep(4);
    printf(">> Since this test is run on the localhost, client1 and client2 will both have the same host number, so both clients commiting on the same project will override the other person's commit\n");
    sleep(10);
    printf(">> This unfortunate situation is mitigated by making sure two clients are run on two DIFFERENT MACHINES\n");
    sleep(5);
    printf(">> We will give 20 second pause so you can inspect the result of commit\n");
    sleep(20);

    //Push the changes on project1 to server repo
    printf(">> Now that we have an active commit lets push those changes\n");
    sleep(5);
    printf(">> Before we do notice that client1 project1's manifest's version number has increased when we ran commit\n");
    sleep(5);
    printf(">> The version of each file we are committing also increased by 1 if we were trying to Add (A) or Modify (M) it to the server\n");
    sleep(8);
    printf(">> I would have liked to implement a solution that would only increment the project manifest when a commit has been successfully pushed...but the project description said to increase on commit not on push\n");
    sleep(10);
    printf(">> The unfortunate result of this is that if an active commit is deleted by the server repo, the local manifest version will still be incremented\n");
    sleep(5);
    printf(">> The good news is that you can't commit the new manifest with the higher version because you will first need to run update which will revert all the changes so that they match\n");
    sleep(8);
    printf(">> Okay lets push the commit by running \"./client1/WTF push project1\" in 5 seconds\n");
    sleep(5);
    system("./client1/WTF push project1");
    printf(">> WE SUCCESSFULLY PUSHED OUR CHANGES!\n");
    sleep(5);
    printf(">> We will give 20 seconds for you to see that all the files were added to the server project1 and the server manifest for project1 was also updated!\n");
    sleep(20);

    //Checkout project1 to client2
    printf(">> Now lets demonstrate checkout by checking out project1 to client2\n");
    sleep(5);
    printf(">> This command is pretty straight forward it copies all files listed from the server's project's manifest over to the client\n");
    sleep(5);
    printf(">> It ONLY transfers files listed in the manifest and to show that we will create another random file in project1 on the server but this time we won't add it to the manifest\n");
    sleep(10);
    system("touch ./server/project1/dontsend.txt");
    system("echo 'Please dont send me' > ./server/project1/dontsend.txt");
    printf(">> We just created the dontsend.txt file now lets run \"./client2/WTF checkout project1\" in 5 seconds\n");
    sleep(5);
    system("./client2/WTF checkout project1");
    printf(">> Client2 just ran checkout\n");
    sleep(3);
    printf(">> We will give 20 seconds to see that ALL files listed in the manifest were transfered over, ie. dontsend.txt did not transfer\n");
    sleep(20);

    //Add some files to project1 in client2
    printf(">> Now that project1 is in client2 let's edit some of the files and commit and push back to the server\n");
    sleep(5);
    printf(">> For this commit we will make sure we have 'M' 'A' and 'D' as operations to perfrom when committing and pushing\n");
    sleep(4);
    printf(">>We will modify the text in america.txt from 'Is the best country in the world' to 'Canada is the best country'\n");
    sleep(3);
    printf(">> We will remove example.txt from the manifest so that we get a 'D' operation\n");
    sleep(4);
    printf(">> We will create a new file 'justadded.txt' with random text and add it to the manifest to get a 'A' operation\n");
    sleep(4);
    printf(">> Ready?\n");
    sleep(2);
    system("echo 'Canada is the best country' > ./client2/project1/america.txt"); //M
    system("./client2/WTF remove project1 example.txt"); //D
    system("touch ./client2/project1/subdir/justadded.txt");
    system("echo 'nothing important in here' > ./client2/project1/subdir/justadded.txt");
    system("./client2/WTF add project1 ./client2/project1/subdir/justadded.txt"); //A
    printf(">> Okay the files have just been added and 'and' and 'remove' commands were used to add and remove the files from the manifest, we will give 15 seconds to inspect\n");
    sleep(15);

    //Commit and push changes to server
    printf(">> Okay time to commit and push changes to the server in 5 seconds, you already know the command now :)\n");
    sleep(5);
    system("./client2/WTF commit project1");
    printf(">> Client2 just commited, we will wait 20 seconds so that you can check the .Commit files to see the different operations\n");
    sleep(20);
    printf(">> Okay time to push the commit in 5 seconds\n");
    system("./client2/WTF push project1");
    sleep(5);
    printf(">> Client2 just pushed project2's commit to the server!\n");
    sleep(3);
    printf(">> We will wait 20 seconds so that you make inspect that the files were changed\n");
    sleep(20);
    printf(">> Checked? Before we move on note a couple things\n");
    sleep(3);
    printf(">> Note that the server manifest version incremented and so have the file versions and hashs\n");
    sleep(4);
    printf(">> The main thing to notice is that example.txt is no longer in the manifest, justadded.txt is, and america.txt just changed\n");
    sleep(5);
    printf(">> Also note that shakespeare remained the same throughout the commit because the shakespeare on the client side was not changed so we don't need to commit that file\n");
    sleep(7);
    printf(">> Alright now that we showed commit works, lets show that update and upgrade work too by pulling these recent version of project1 to client1\n");
    sleep(8);

    //Update and upgrade project1 on client1 CONFLICT
    printf(">> Before we show update, lets describe what we expect should happen\n");
    sleep(3);
    printf(">> Since client2 edited america.txt we should expect that file to be updated with a 'M' operation\n");
    sleep(4);
    printf(">> Client2 also deleted example.txt from project1, so we should expect client1 to reflect this change with a 'D' operation\n");
    sleep(5);
    printf(">> Client2 also added justadded.txt so we should expect client1 to receive this file too with a 'A' operation\n");
    sleep(5);
    printf(">> First lets show a conflict, by changing america.txt on client1 so the live hash and the hash in client1's manifest mismatch\n");
    sleep(5);
    system("echo 'changed' > ./client1/project1/america.txt");
    printf(">> america.txt just changed, inspect if you wish we will wait 10 seconds\n");
    sleep(10);
    printf(">> Lets run ./client1/WTF update project1 in 5 seconds\n");
    sleep(5);
    system("./client1/WTF update project1");
    printf(">> We just tried running update but since there was conflict no .Update file was created and conflicts are written to .Conflict, you can inspect this .Conflict file to see the confict, we will wait 20 seconds\n");
    sleep(20);

    //Update and upgrade project1 on client1
    printf(">> Lets fix and the conflict by reverting america.txt to what it was before and rerun update\n");
    sleep(5);
    system("echo 'Is the best country in the world' > ./client1/project1/america.txt");
    printf(">> america.txt has been reverted, lets run update again in 5 seconds\n");
    sleep(5);
    system("./client1/WTF update project1");
    printf(">> We just ran update and there should be .Update file in client1 project1 detailing all the operations we need to do described above, we will have 20 seconds so you can inspect .Update\n");
    sleep(20);
    printf(">> Now lets run upgrade and see america.txt change, example.txt be deleted, and justadded.txt be added to the project\n");
    sleep(6);
    printf(">> We will run ./client1/WTF upgrade project1 in 5 seconds\n");
    sleep(5);
    system("./client1/WTF upgrade project1");
    printf(">> Client1 project1 was just upgraded!!\n");
    sleep(5);
    printf(">> We will wait 20 seconds so you can see the update, note that the .Update file is now deleted and the server and client manifest match\n");
    sleep(20);

    //Commit and push one more time by changing justadded.txt
    printf(">> Now it's time to show the currentversion command\n");
    sleep(5);
    printf(">> Before we show the currentversion command, lets edit justadded.txt on client1 project1 and push the changes to the server\n");
    sleep(3);
    printf(">> We will do this so that so more information in the .history file, and so we can demostrate rollback later\n");
    sleep(5);
    printf(">> In 10 seconds we will edit justadded.txt from 'Nothing important in here' to 'Very important stuff in here' , commit, and push the changes to the server\n");
    sleep(10);
    system("echo 'Very important stuff in here' > ./client1/project1/subdir/justadded.txt");
    system("./client1/WTF commit project1");
    system("./client1/WTF push project1");
    printf(">> We just commited and push project1! Inspect the server manifest for project1 and the justadded.txt file to see if it changed, we will wait 20 seconds\n");
    sleep(20);

    //Show currentversion from client2
    printf(">> Now let us show the currentversion command, which we will use client2 to perform\n");
    sleep(4);
    printf(">> In 10 seconds we will run './client2/WTF currentversion project1' which will list out all the files currently under version control by the server\n");
    sleep(10);
    system("./client2/WTF currentversion project1");
    printf(">> Current version just ran! If you look above you can see the output of the command, which lists the files under control, we will wait 15 seconds for you to read the output\n");
    sleep(15);

    //Show history
    printf(">> Now lets show the history command which client2 will also perform\n");
    sleep(4);
    printf(">> The history command uses the .history file stored in the server version of the project\n");
    sleep(4);
    printf(">> The history command lists every successful push made to the project on the server\n");
    sleep(4);
    printf(">> In 5 seconds we will run ./client2/WTF history project1\n");
    sleep(5);
    system("./client2/WTF history project1");
    printf(">> The history command just ran! Inspect the output above to see all the successful operations performed on files with the manifest version of the push listed first. We will give 15 seconds to inspect the output\n");
    sleep(15);

    //Perform rollback
    printf(">> Its time to showcase our final command, 'rollback'\n");
    sleep(5);
    printf(">> We will rollback project1 to version 2, and use client2 to do so\n");
    sleep(4);
    printf(">> The way the server holds old versions of the project is by duplicating the directory and appending the old project version to its name on every successfull push, ie <projectname>_<oldversion>\n");
    sleep(7);
    printf(">> Lets create 'fake' rollbacks to make sure the 'rollbacl' command doesn't delete wrong rollback versions\n");
    sleep(5);
    printf(">> We will create 'project2_2' as a fake project rollback in 2 seconds...post rollback note that this file remains intact\n");
    sleep(2);
    system("mkdir ./server/project2_2");
    printf(">> We will run './client2/WTF rollback project1 2' in 5 seconds\n");
    sleep(5);
    system("./client2/WTF rollback project1 2");
    printf(">> The project has been successfully rollbacked! The old manifest and every file in the old version is now the current version\n");
    sleep(6);
    printf(">> When rolling back to a version, all higher versions of the project are deleted\n");
    sleep(5);
    printf(">> You can see this because project1_3 was deleted but project1_1 remained when we rolled back to version 2\n");
    sleep(5);
    printf(">> We will give 20 seconds to see that the project succesfully rolled backed\n");
    sleep(20);

    //Update client1 to the new rolled backed edition
    printf(">> Now just for the sake of completeness, lets update and upgrade client1 project1 to the new rollbacked version since right now it's on a higher version\n");
    sleep(10);
    printf(">> In 10 seconds we will run update and upgrade, give notice to the current state of client1 project1 and the state after\n");
    sleep(10);
    system("./client1/WTF update project1");
    system("./client1/WTF upgrade project1");
    printf(">> Client1 had successfully updated its project to the rollbacked version! We will give 20 seconds to see for yourself\n");
    sleep(20);

    //Thank you
    printf(">> Thank you for sticking through this WTFtest and seeing how our version of git\n");
    sleep(5);
    printf(">> Pls give us a 100 Thank you so much, Goodbye...\n");
    sleep(5);
    kill(server, SIGKILL);
    system("make delete");
    return 0;
}