use embassy_sync::channel::DynamicSender;
use core::cell::RefCell;
use core::ops::Shr;
use alloc::rc::Rc;
use alloc::vec::Vec;



pub struct Source<'a,T> where T:Clone{
    senders : Rc<RefCell<Vec<DynamicSender<'a,T>>>>
}

pub trait SourceTrait<'a,T> where T:Clone{
    fn add_sender(&self, sender: DynamicSender<'a,T>);
    fn emit(&self, msg: &T);
}

impl <'a,T> Source<'a,T> where T:Clone {
    pub fn new() -> Self {
        Self {
            senders: Rc::new(RefCell::new(Vec::new()))
        }
    }
}

impl <'a,T> SourceTrait<'a,T> for Source<'a,T> where T:Clone{
     fn add_sender(&self, sender: DynamicSender<'a,T>) {
        self.senders.borrow_mut().push(sender);
    }
     fn emit(&self, msg: &T) {
        for sender in self.senders.borrow().iter() {
            let _ = sender.try_send(msg.clone());
        }
    }
}

impl<'a,T> Shr<&'a DynamicSender<'a,T>> for &mut Source<'a,T> where T:Clone {
    type Output = ();
    fn shr(self, rhs: &'a DynamicSender<T>) -> Self::Output {
        self.add_sender(rhs.clone());
    }
}