for f in {31..35}; do
    curl https://cdn.robovinci.xyz/imageframes/${f}.initial.json -o initial/${f}.json
    curl https://cdn.robovinci.xyz/imageframes/${f}.png -o images/${f}.png
    curl https://cdn.robovinci.xyz/imageframes/${f}.initial.png -o images/${f}.initial.png
done
