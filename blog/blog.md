Sure! here is a well-crafted ... wait a minute. This is not written by GPT! 100% organic. 100% stupid.
This is a tale of consistently falling down rabbit holes, and my love for high-perf code for which I have
been constantly nerd-sniped, be it with or without my consent.

This was actually going to be a section in a much bigger blog post. But I looked at my analytics and I figured
that people don't stick around for more than 3 minutes on the post. Welp. That means no one is reading 3000 words I write.
Which means I need smaller blog posts -- or an audience with a longer attention span. Considering myself to be a person
who struggles with the latter these days - a lot - I guess the first is the way to go.

## Mission, vision.

Our mission and vision this time around is to write a Map. A very performant one, tailored to our specific needs. For this,
we have everything at our disposal - alas - except any third party libraries. Which means, we get a C++20 compiler, and
hopes and dreams. Oh and also, we get to peek at the data. At least at the top 20% of the total actionable data anyways (I 
did this to prevent myself from writing a generic solution).

We have a file with 1 billion rows ([oh boy here we go again](insert link here)). The task is to create a hashmap, so that, for each row that contains a city(string);measurement(floating point number - 1 digit precision), we add the measurement to the hashmap. Just to make things easier, we are allowed to work on a dataset for now, with 1 billion rows, but each time we write to the hashmap, the previous value for the city can be overwritten (so that I can solely focus on the task of optimizing the map, instead of struggling with scaled ints).

TL;DR: Have 1 billion pairs (city, number), add all pairs to map, overwrite (city, number_old) if (city, number_old) already in map.

## Baseline

Every good performance story needs a good baseline. Fortunately, for me, the task of generating the dataset and the baseline can be done in a single benchmark binary in C++.