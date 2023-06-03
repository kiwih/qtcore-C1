# Chats used to make a processor

In this directory are the conversations I had with ChatGPT (GPT-4) which were used to make first the QTcore-A1 processor which was then adapted (re-made, really) to be the QTcore-C1 processor.

The first thing to note is that neither of the processors were not made in a single chat session. 
Instead, they were made across multiple conversations, with only one or two topics being addressed in each. 
This was a concious decision made for two reasons: (1) the language models have a finite input/output context window, so breaking up via topics presents the models from getting lost; and (2) it also helped keep the design process organized in much the same way you as a human might organize your own development flow.

Secondly, while the development broadly flowed linearly from topic to topic, there are interdependencies. This is because sometimes development in one conversations requires updates in others, especially when bugs or defects are found. It is usually quicker to continue an existing conversation rather than build a whole new one. 

Finally, the conversations do not specifically result in files. Instead, they produced Verilog modules or snippets of modules that I as an engineer copied and pasted in the logical order to build up the processor itself. This is very much a co-design process, so in addition to the feedback I gave the model, I also had to handle the extraction of the code and giving it to the tooling (Xilinx Vivado and IVerilog). Overall the model wrote 100% of all code aside from the top-level I/O, which I specified according to the needs of Tiny Tapeout (QTcore-A1) and Caravel (CTcore-C1)

- Hammond Pearce, June 2023

