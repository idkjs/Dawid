type slideState = Slide | SlideDown | Inactive;

type slide = {
  isShown: bool,
  node: Dom.node,
  slideState: slideState,

  heading: string,
  subheading: string
};

type state = {
  slides: array(slide),
  container: Dom.node,
  observer: Observable.observer(state, string),
  observable: Observable.observable(state, string)
};

let clear = (state: state): state => {
  let slides = Array.map(s => {
    Dom.removeChild(state.container, s.node);
    {...s, isShown: false}
  }, state.slides);

  {...state, slides};
};

let clearState = (slides: list(slide)): list(slide) => {
  List.map(slide => {
    switch (slide.slideState) {
      | Slide => Dom.classListRemove(slide.node, "slide")
      | SlideDown => Dom.classListRemove(slide.node, "slide-down")
      | Inactive => ()
    };

    {...slide, slideState: Inactive};
  }, slides);
};

let showImage = (state: state, idx: int): state => {
  Dom.appendChild(state.container, state.slides[idx].node);
  state.slides[idx] = {...state.slides[idx], isShown: true};

  state;
};

/* TODO: Use images, not indices */

let transitionBackwards = (state: state, idx: int): state => {
  switch (Js_array.find(s => s.isShown, state.slides)) {
    | None => {
      state.observer#error("Cannot transition because no slide is shown");
      state;
    }
    | Some(shownSlide) => {
      let slideToShow = state.slides[idx];
      let shownSlideIdx = Js_array.findIndex((===)(shownSlide), state.slides);

      Dom.classListAdd(shownSlide.node, "slide-down");
      Dom.classListAdd(slideToShow.node, "slide-down");
      Dom.insertBefore(state.container, slideToShow.node, shownSlide.node);

      state.slides[shownSlideIdx] = {...shownSlide, slideState: SlideDown};
      state.slides[idx] = {...slideToShow, isShown: true, slideState: SlideDown};

      let rec animationEndCb = () => {
        Dom.removeChild(state.container, shownSlide.node);
        state.slides[shownSlideIdx] = {...shownSlide, isShown: false, slideState: Inactive};
        state.slides[idx] = {...state.slides[idx], slideState: Inactive};
        state.observer#next(state);

        Dom.removeEventListener(slideToShow.node, Dom.animationEnd, animationEndCb);
      };

      Dom.addEventListener(slideToShow.node, Dom.animationEnd, animationEndCb, false);
      state;
    }
  };
};

let create = (selector: string): state => {
  let container = Dom.querySelector(Dom.document, selector);
  let slideNodes: array(Dom.node) = [%bs.raw {| container.children |}];

  let (observer, observable) = Observable.create();

  let slides = Array.map(slide => {
    {
      isShown: false,
      node: slide,
      slideState: Inactive,
      heading: "",
      subheading: ""
    }
  }, slideNodes);

  clear({container, slides, observer, observable});
};
