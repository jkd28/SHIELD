prompt =  'Please enter the transmitted file name: ';
fileNameI = input(prompt,'s');
textin = fileread(fileNameI);
textin = deblank(textin);
bytesin = unicode2native(textin,'UTF-8');

prompt =  'Please enter the received file name: ';
fileNameO = input(prompt,'s');
textout = fileread(fileNameO);
textout = deblank(textout);
bytesout = unicode2native(textout,'UTF-8');

[number1,ratio1] = biterr(bytesin,bytesout)