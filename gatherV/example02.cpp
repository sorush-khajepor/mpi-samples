/*

In this example, all cpus sending a string message to root.
Root print the message on screen.
The length of each cpu message is different. 

Note, here I assume you are using C++11 or above which enforces std::string
to be contiguous.
*/

#include <iostream>
#include <vector>
#include <numeric> // std::accumulate
#include <mpi.h>
using namespace std;

int main()
{
    MPI_Init(NULL, NULL);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Lets mock a real simple system
    string message;
    if (rank == 0)
        message = "Hello :), I'm Rank" + to_string(rank) + "\n";
    else if (rank == 1)
        message = "This is coming from me, Rank" + to_string(rank) + "\n";
    else if (rank == 2)
        message = "Yes, I'm here too, Rank" + to_string(rank) + "\n";

    // Sending message length of each cpu to root
    int nlocal = message.length();

    std::vector<int> recvLenghts;
    if (rank == 0)
        recvLenghts.resize(size);

    MPI_Gather(&nlocal, 1, MPI_INT, recvLenghts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // lets print recieve lengths by root
    if (rank == 0) cout << "Receive lengths:\n";
    for (size_t i = 0; i < recvLenghts.size(); i++)
    {
        cout << recvLenghts[i] << " ";
    }
    if (rank == 0) cout << "\n";

    // Now we have to compute displacements, and total message length
    int totlen = 0;
    vector<int> displs;
    string totalMessage;

    if (rank == 0)
    {
        totlen  = std::accumulate(recvLenghts.begin(), recvLenghts.end(), 0);
        totalMessage.resize(totlen);
        cout << "\ntotal length =" << totlen << "\n";


        displs.resize(size);
        displs[0] = 0;
        for (int i = 1; i < size; i++)
        {
            displs[i] = displs[i - 1] + recvLenghts[i - 1];
        }


        cout << "\ndisplacements are:\n";
        for (auto &&item : displs)
        {
            cout << item << " ";
        }

    }

    // And finally,
    MPI_Gatherv(&message[0], message.length(), MPI_CHAR,
                &totalMessage[0], recvLenghts.data(), displs.data(), MPI_CHAR,
                0, MPI_COMM_WORLD);

    if (rank == 0)
        cout << "Total message at rank0 = \n";
    // The intersting thing here is that all
    // the CPUs run this part, but only root
    // has totalMessage. The others write nothing.
    cout << totalMessage << "\n";

    MPI_Finalize();
}